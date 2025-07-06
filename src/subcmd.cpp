#include "subcmd.h"
#include "plugin/loader.h"
#include "log.h"
#include "toml/default/default.h"
#include <algorithm>
#include "res.h"
#include "utils/utils.h"
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
    auto plugin = PluginLoader(this->type);
    LOG_INFO("Creating new project ", this->name, " with type ", this->type);
    std::optional<std::string> except = std::nullopt;
    auto plugin_name = plugin->getName(except);
    if (except)
        throw std::runtime_error(*except);
    LOG_INFO("Execute by ", plugin_name);
    const auto project_dir = this->root / this->name;
    LOG_DEBUG(project_dir);
    if (fs::exists(project_dir))
        throw std::runtime_error("The directory '" + this->name + "' already exists.");

    except = std::nullopt;
    auto result = plugin->run_new({
                                      .name = this->name,
                                      .type = this->type,
                                      .root = this->root,
                                  },
                                  except);
    if (except)
    {
        fs::remove_all(this->root / this->name);
        throw std::runtime_error(*except);
    }
    return result;
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
    else if (this->key.starts_with("@"))
    {
        auto plugin_name = this->key.substr(1);
        auto plugin = PluginLoader(plugin_name);
        std::optional<std::string> except = std::nullopt;
        plugin->show_help(this->args, except);
        if (except)
            throw std::runtime_error(*except);
    }
    else
        throw std::runtime_error("No help information for '" + this->key + "'.");
    return 0;
}

Version::Version(const cmd::Args &args) : SubCommand(args) {}

int Version::run()
{
    std::cout << "Cup version " << _VER_X << "." << _VER_Y << "." << _VER_Z << std::endl;
    return 0;
}

Build::Build(const cmd::Args &args) : SubCommand(args)
{
    if (args.has_config("dir") && args.getConfig().at("dir").size() > 0)
        this->root = args.getConfig().at("dir")[0];
    else
        this->root = ".";
    if (this->root.is_relative())
        this->root = fs::current_path() / this->root;
    this->is_release = args.has_flag("release") || args.has_flag("r");
    if (args.getPositions().size() >= 2)
        this->command = args.getPositions()[1];
}

using VersionInfo = std::tuple<int, int, int>;
VersionInfo parse_version(const std::string &version)
{
    auto parts = split(version, ".");
    if (parts.size() != 3)
        throw std::runtime_error("version '" + version + "' is not a valid version string.");
    return {std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2])};
}

std::pair<fs::path, std::string> get_path(
    const data::Dependency &dep, bool download = true,
    const std::optional<fs::path> &root = std::nullopt)
{
    if (dep.path && dep.url)
        std::runtime_error("Both 'path' and 'url' are specified.");
    if (dep.path)
    {
        auto path = *dep.path;
        if (path.is_relative() && root.has_value())
            path = *root / path;
        path.lexically_normal();
        auto toml_config = data::Deserializer<data::Default>::deserialize(
            toml::parse_file((path / "cup.toml").string()));
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

struct DependencyInfo
{
    fs::path path;
    std::string type;
    VersionInfo version;
    std::vector<std::string> features;
};

void _get_all_dependencies(
    const data::Default &toml_config, std::vector<std::pair<std::string, DependencyInfo>> &dependencies,
    const std::optional<fs::path> &root = std::nullopt)
{
    for (const auto &[name, info] : toml_config.dependencies.value_or(std::map<std::string, data::Dependency>{}))
    {
        auto [path, version] = get_path(info, true, root);
        if (!fs::exists(path))
            throw std::runtime_error("Dependency '" + name + "' not found.");
        auto dep_config = data::Deserializer<data::Default>::deserialize(
            toml::parse_file((path / "cup.toml").string()));
        auto dep_info = DependencyInfo{
            .path = path,
            .type = dep_config.project.type,
            .version = parse_version(version),
            .features = info.features.value_or(std::vector<std::string>{}),
        };
        _get_all_dependencies(dep_config, dependencies, path);
        decltype(dependencies.begin()) iter;
        if ((iter = std::find_if(dependencies.begin(), dependencies.end(), [&](const auto &item)
                                 { return item.first == name; })) != dependencies.end())
        {
            auto &exist_info = iter->second;
            if (std::get<0>(exist_info.version) < std::get<0>(dep_info.version) ||
                (std::get<0>(exist_info.version) == std::get<0>(dep_info.version) &&
                 std::get<1>(exist_info.version) < std::get<1>(dep_info.version)) ||
                (std::get<0>(exist_info.version) == std::get<0>(dep_info.version) &&
                 std::get<1>(exist_info.version) == std::get<1>(dep_info.version) &&
                 std::get<2>(exist_info.version) < std::get<2>(dep_info.version)))
            {
                iter->second = dep_info;
            }
        }
        else
        {
            dependencies.push_back({name, dep_info});
        }
    }
}

std::vector<std::pair<std::string, DependencyInfo>> get_all_dependencies(const data::Default &toml_config,
                                                                         const std::optional<fs::path> &root = std::nullopt)
{
    std::vector<std::pair<std::string, DependencyInfo>> dependencies;
    _get_all_dependencies(toml_config, dependencies, root);
    return dependencies;
}

int Build::run()
{
    // Parse cup.toml file.
    auto toml_config = data::Deserializer<data::Default>::deserialize(
        toml::parse_file((this->root / "cup.toml").string()));
    // Get all dependencies.
    CMakeContext context{
        .name = toml_config.project.name,
        .cmake_version = {3, 10},
        .current_dir = this->root,
        .root_dir = this->root,
    };
    // Generate cmake content.
    auto dependencies = get_all_dependencies(toml_config, this->root);
    std::vector<std::string> cmake_content;
    std::optional<std::string> except = std::nullopt;
    for (const auto &[name, info] : dependencies)
    {
        LOG_INFO("Generating cmake content for dependency '", name, "' with type '", info.type, "'.");
        PluginLoader loader(info.type);
        context.name = name;
        context.current_dir = info.path;
        context.features = info.features;
        except = std::nullopt;
        cmake_content.push_back(loader->gen_cmake(context, true, except));
        if (except)
            throw std::runtime_error(*except);
    }
    auto end_name = toml_config.project.name;
    auto end_type = toml_config.project.type;
    LOG_INFO("Generating cmake content for ", end_name, " with type ", end_type);
    auto loader = PluginLoader(end_type);
    context.name = end_name;
    context.current_dir = this->root;
    except = std::nullopt;
    cmake_content.push_back(loader->gen_cmake(context, false, except));
    if (except)
        throw std::runtime_error(*except);
    // Write cmake content to file.
    {
        if (!fs::exists(Resource::build(this->root)))
            fs::create_directories(Resource::build(this->root));
        auto cmakelists = Resource::build(this->root) / "CMakeLists.txt";
        std::ofstream ofs(cmakelists);
        ofs << "# Generated by cup\n";
        auto [major, minor] = context.cmake_version;
        ofs << "cmake_minimum_required(VERSION " << major << "." << minor << ")\n";
        ofs << "project(" << context.name << ")\n";
        if (toml_config.build.has_value() &&
            toml_config.build->export_data.has_value() &&
            toml_config.build->export_data->compile_commands.has_value())
        {
            ofs << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
        }
        if (this->is_release)
        {
            ofs <<
#include "template/release.cmake.txt"
                << std::endl;
        }
        else
        {
            ofs <<
#include "template/debug.cmake.txt"
                << std::endl;
        }
        for (const auto &content : cmake_content)
            ofs << content << "\n";
    }

    LOG_MSG("Building...");
    {
        cmd::CMake cmake;
        cmake.source(Resource::build(this->root));
        cmake.build_dir(Resource::build(this->root) / "cmake");
        if (toml_config.build.has_value() && toml_config.build->generator.has_value())
            cmake.generator(*toml_config.build->generator);
        else
        {
#ifdef _WIN32
            cmake.generator("Visual Studio 17 2022");
            LOG_WARN("Using default generator 'Visual Studio 17 2022'.");
#else
            cmake.generator("Unix Makefiles");
            LOG_WARN("Using default generator 'Unix Makefiles'.");
#endif
        }
        if (std::system(cmake.as_command().c_str()))
            throw std::runtime_error("Failed to generate build files.");
    }
    {
        cmd::CMake cmake;
        cmake.build(Resource::cmake(this->root));
        cmake.config(this->is_release);
        std::optional<std::string> except = std::nullopt;
        auto target = loader->get_target(RunProjectData{
                                             .command = this->command,
                                             .root = this->root,
                                             .name = toml_config.project.name,
                                             .is_debug = !this->is_release,
                                         },
                                         except);
        if (except)
            throw std::runtime_error(*except);
        if (target)
            cmake.target(*target);
        if (std::system(cmake.as_command().c_str()))
            throw std::runtime_error("Failed to build project.");
    }
    if (toml_config.build.has_value() &&
        toml_config.build->export_data.has_value() &&
        toml_config.build->export_data->compile_commands.has_value())
    {
        auto compile_commands = toml_config.build->export_data->compile_commands.value();
        auto from = Resource::cmake(this->root) / "compile_commands.json";
        if (!fs::exists(from))
        {
            auto gen = toml_config.build->generator.value_or(
#ifdef _WIN32
                "Visual Studio 17 2022"
#else
                "Unix Makefiles"
#endif
            );
            LOG_WARN("Generator \"", gen, "\" does not support compile_commands.json.");
        }
        if (!compile_commands.empty() && fs::exists(from))
        {
            if (compile_commands.is_relative())
                compile_commands = this->root / compile_commands;
            compile_commands = compile_commands.lexically_normal();
            auto to = compile_commands / "compile_commands.json";
            if (!fs::exists(compile_commands))
                throw std::runtime_error("The directory '" + compile_commands.string() + "' does not exist.");
            fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        }
    }
    LOG_MSG("Build finished.");
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
}

int Clean::run()
{
    auto cup_toml = this->root / "cup.toml";
    if (!fs::exists(cup_toml))
        throw std::runtime_error("The directory '" + this->root.string() + "' is not a cup project.");
    auto build_dir = this->root / "target" / "build";
    LOG_INFO("Cleaning build directory...");
    if (fs::exists(build_dir))
        fs::remove_all(build_dir);
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
    auto toml_config = data::Deserializer<data::Default>::deserialize(
        toml::parse_file((this->root / "cup.toml").string()));
    RunProjectData data{
        .command = this->command,
        .root = this->root,
        .name = toml_config.project.name,
        .is_debug = !this->is_release,
    };
    auto loader = PluginLoader(toml_config.project.type);
    std::optional<std::string> except = std::nullopt;
    this->target = loader->get_target(data, except);
    if (except)
        throw std::runtime_error(*except);
    auto ret = Build::run();
    if (ret)
        return ret;
    except = std::nullopt;
    auto path = loader->run_project(data, except);
    if (except)
        throw std::runtime_error(*except);
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
