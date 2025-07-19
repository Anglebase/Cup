#include "subcmd.h"
#include "plugin/loader.h"
#include "log.h"
#include "toml/default/default.h"
#include <algorithm>
#include "res.h"
#include "utils/utils.h"
#include "plugin/built-in/utils.h"
#include <tuple>
#include "cmd/cmake.h"
#include "cmd/git.h"
#include <functional>

New::New(const cmd::Args &args) : SubCommand(args)
{
    if (args.getPositions().size() < 2)
        throw std::runtime_error("Project name not specified.");
    auto name = args.getPositions()[1];
    // Check if project name is valid.
    // Project name should only contain alphanumeric characters and underscore.
    if (std::find_if_not(name.begin(), name.end(), [](char c)
                         { return std::isalnum(c) || c == '_'; }) != name.end())
        throw std::runtime_error("'" + name + "is not a valid project name.");
    this->name = name;
    // Get project type from config file.
    // If type is not specified, default to binary.
    // The built-in supported project types include binary, shared, static, and module.
    if (args.has_config("type") && args.getConfig().at("type").size() > 0)
        this->type = args.getConfig().at("type")[0];
    else
        this->type = "binary";

    if (args.has_config("dir") && args.getConfig().at("dir").size() > 0)
        this->root = args.getConfig().at("dir")[0];
    else
        this->root = ".";
    if (this->root.is_relative())
        this->root = fs::current_path() / this->root;
}

int New::run()
{
    LOG_MSG("Creating...");
    auto plugin = PluginLoader(this->type);
    LOG_INFO("Creating new project '", this->name, "' with type '", this->type, "'.");
    auto plugin_name = plugin->getName();
    if (plugin_name.is_error())
        throw std::runtime_error(plugin_name.error());
    LOG_INFO("Execute by ", plugin_name.ok());
    const auto project_dir = this->root / this->name;
    LOG_DEBUG(project_dir);
    if (fs::exists(project_dir))
        throw std::runtime_error("The directory '" + this->name + "' already exists.");

    auto result = plugin->run_new({
                                      .name = this->name,
                                      .type = this->type,
                                      .root = this->root,
                                  });
    if (result.is_error())
    {
        fs::remove_all(this->root / this->name);
        throw std::runtime_error(result.error());
    }
    LOG_MSG("Finished.");
    return result.ok();
}
const std::unordered_map<std::string, std::string> Help::help_info = {
    {
        "",
#include "template/help/all.txt"
    },
    {
        "help",
#include "template/help/help.txt"
    },
    {
        "build",
#include "template/help/build.txt"
    },
    {
        "clean",
#include "template/help/clean.txt"
    },
    {
        "install",
#include "template/help/install.txt"
    },
    {
        "uninstall",
#include "template/help/uninstall.txt"
    },
    {
        "list",
#include "template/help/list.txt"
    },
    {
        "new",
#include "template/help/new.txt"
    },
    {
        "run",
#include "template/help/run.txt"
    },
};

Help::Help(const cmd::Args &args) : SubCommand(args), args(args)
{
    if (args.getPositions().size() >= 2)
        this->key = args.getPositions()[1];
    else
        this->key = "";
}

int Help::run()
{
    if (Help::help_info.contains(this->key))
        std::cout << Help::help_info.at(this->key) << std::endl;
    else if (this->key.starts_with("/"))
    {
        auto plugin_name = this->key.substr(1);
        auto plugin = PluginLoader(plugin_name);
        auto result = plugin->show_help(this->args);
        if (result.is_error())
            throw std::runtime_error(result.error());
        return result.ok();
    }
    else
        throw std::runtime_error("No help information for '" + this->key + "'.");
    return 0;
}

Version::Version(const cmd::Args &args) : SubCommand(args) {}

int Version::run()
{
    std::cout << "Cup version v" << _VER_X << "." << _VER_Y << "." << _VER_Z << std::endl;
    return 0;
}

List::List(const cmd::Args &args) : SubCommand(args)
{
    if (args.getPositions().size() < 2)
        throw std::runtime_error("No option specified.");
    this->option = args.getPositions()[1];
}

int List::run()
{
    std::unordered_map<std::string, std::function<int(void)>> options = {
        {
            "plugins",
            []()
            {
                LOG_INFO("Installed plugins:");
                std::cout << "\t- " << "binary*" << std::endl;
                std::cout << "\t- " << "static*" << std::endl;
                std::cout << "\t- " << "shared*" << std::endl;
                std::cout << "\t- " << "module*" << std::endl;
                if (fs::exists(Resource::plugins()))
                    for (const auto &entry : fs::directory_iterator(Resource::plugins()))
                        if (entry.is_regular_file())
                            std::cout << "\t- " << entry.path().stem().string() << std::endl;
                return 0;
            },
        },
        {
            "packages",
            []()
            {
                if (!fs::exists(Resource::packages()) || fs::is_empty(Resource::packages()))
                {
                    LOG_INFO("No packages have been installed yet.");
                    return 0;
                }
                LOG_INFO("Installed packages:");
                for (const auto &entry : fs::directory_iterator(Resource::packages()))
                    if (entry.is_directory())
                        std::cout << entry.path().filename().string() << std::endl;
                return 0;
            },
        },
    };
    if (options.contains(this->option))
        return options.at(this->option)();
    else
        throw std::runtime_error("Invalid option.");
}

Install::Install(const cmd::Args &args) : SubCommand(args)
{
    if (args.getPositions().size() < 2)
        throw std::runtime_error("Dependency url not specified.");
    this->url = args.getPositions()[1];
    if (args.has_config("version") && args.getConfig().at("version").size() > 0)
        this->version = args.getConfig().at("version")[0];
}

int Install::run()
{
    get_path(data::Dependency{
        .version = this->version,
        .url = this->url,
    });
    return 0;
}

Uninstall::Uninstall(const cmd::Args &args) : SubCommand(args)
{
    if (args.getPositions().size() < 2)
        throw std::runtime_error("Dependency url not specified.");
    this->url = args.getPositions()[1];
    if (args.has_config("version") && args.getConfig().at("version").size() > 0)
        this->version = args.getConfig().at("version")[0];
    else
        throw std::runtime_error("Dependency version not specified.");
}

int Uninstall::run()
{
    auto [path, _] = get_path(data::Dependency{
                                  .version = this->version,
                                  .url = this->url,
                              },
                              false);
    if (!fs::exists(path))
        throw std::runtime_error(
            "There is no package with the name " +
            this->url + " and version " + this->version +
            " in the installed package.");
    fs::remove_all(path);
    LOG_INFO("Uninstalled package ", this->url, " with version ", this->version);
    return 0;
}

Clean::Clean(const cmd::Args &args) : SubCommand(args)
{
    if (args.has_config("dir") && args.getConfig().at("dir").size() > 0)
        this->root = args.getConfig().at("dir")[0];
    else
        this->root = ".";
    if (this->root.is_relative())
        this->root = fs::current_path() / this->root;
    this->all = args.has_flag("all");

    if (!fs::exists(this->root))
        throw std::runtime_error("The directory '" + this->root.lexically_normal().string() + "' not exists.");
    if (!fs::exists(this->root / "cup.toml"))
        throw std::runtime_error("The directory '" + this->root.lexically_normal().string() + "' not a cup project.");
}

int Clean::run()
{
    auto cup_toml = this->root / "cup.toml";
    if (!fs::exists(cup_toml))
        throw std::runtime_error("The directory '" + this->root.string() + "' is not a cup project.");
    auto clean_dir = all ? Resource::target(this->root) : Resource::build(this->root);
    LOG_INFO("Cleaning build directory...");
    if (fs::exists(clean_dir))
        fs::remove_all(clean_dir);
    LOG_INFO("Finished...");
    return 0;
}

Run::Run(const cmd::Args &args) : Build(args)
{
    if (args.has_config("args"))
        this->args = join(args.getConfig().at("args"), " ");
}

int Run::run()
{
    auto toml_config = data::parse_toml_file<data::Default>(this->root / "cup.toml");
    RunProjectData data{
        .command = this->command,
        .root = this->root,
        .name = toml_config.project.name,
        .is_debug = !this->is_release,
    };
    auto loader = PluginLoader(toml_config.project.type);
    auto result = loader->get_target(data);
    if (result.is_error())
        throw std::runtime_error(result.error());
    this->target = result.ok();
    auto ret = Build::run();
    if (ret)
        return ret;
    auto result_ = loader->run_project(data);
    if (result_.is_error())
        throw std::runtime_error(result_.error());
    auto path = result_.ok();
    if (path.is_relative())
        path = this->root / path;
    path = path.lexically_normal();
    auto cmd = path.string() + " " + this->args;
    LOG_MSG("# Running: ", path.string());
    ret = std::system(cmd.c_str());
    if (ret)
        LOG_WARN("# Exit Code: ", ret);
    else
        LOG_MSG("# Exit Code: ", ret);
    return 0;
}

std::pair<fs::path, std::string> get_path(const data::Dependency &dep, bool download, const std::optional<fs::path> &root)
{
    if (dep.path && dep.url)
        std::runtime_error("Both 'path' and 'url' are specified.");
    if (dep.path)
    {
        auto path = *dep.path;
        if (path.is_relative() && root.has_value())
            path = *root / path;
        path = path.lexically_normal();
        auto toml_config = data::parse_toml_file<data::Default>(path / "cup.toml");
        if (dep.version && toml_config.project.version != dep.version.value())
            LOG_WARN("Dependency version is not consistent with the version in 'cup.toml'.");
        return {path, toml_config.project.version};
    }
    else if (dep.url)
    {
        auto url = dep.url.value();
        return Resource::repo_dir(url, dep.version, download);
    }
    else
        throw std::runtime_error("Neither 'path' nor 'url' is specified.");
}