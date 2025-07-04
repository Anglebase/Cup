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
    LOG_INFO("Execute by ", plugin->getName());
    const auto project_dir = this->root / this->name;
    LOG_DEBUG(project_dir);
    if (fs::exists(project_dir))
        throw std::runtime_error("The directory '" + this->name + "' already exists.");
    try
    {
        return plugin->run_new({
            .name = this->name,
            .type = this->type,
            .root = this->root,
        });
    }
    catch (const std::exception &e)
    {
        fs::remove_all(this->root / this->name);
        throw e;
    }
}
const std::unordered_map<std::string, std::string> Help::help_info = {
    {
        "",
#include "template/help/help.txt"
    },
};

Help::Help(const cmd::Args &args) : SubCommand(args)
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
}

using VersionInfo = std::tuple<int, int, int>;
VersionInfo parse_version(const std::string &version)
{
    auto parts = split(version, ".");
    if (parts.size() != 3)
        throw std::runtime_error("version '" + version + "' is not a valid version string.");
    return {std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2])};
}

std::pair<fs::path, std::string> get_path(const data::Dependency &dep, bool download = true)
{
    if (dep.path && dep.url)
        std::runtime_error("Both 'path' and 'url' are specified.");
    if (dep.path)
    {
        auto toml_config = data::Deserializer<data::Default>::deserialize(
            toml::parse_file((dep.path.value() / "cup.toml").string()));
        if (dep.version && toml_config.project.version != dep.version.value())
            LOG_WARN("Dependency version is not consistent with the version in 'cup.toml'.");
        return {dep.path.value(), toml_config.project.version};
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

void _get_all_dependencies(const data::Default &toml_config, std::vector<std::pair<std::string, DependencyInfo>> &dependencies)
{
    for (const auto &[name, info] : toml_config.dependencies.value_or(std::map<std::string, data::Dependency>{}))
    {
        auto [path, version] = get_path(info);
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
        _get_all_dependencies(dep_config, dependencies);
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

std::vector<std::pair<std::string, DependencyInfo>> get_all_dependencies(const data::Default &toml_config)
{
    std::vector<std::pair<std::string, DependencyInfo>> dependencies;
    _get_all_dependencies(toml_config, dependencies);
    return dependencies;
}

int Build::run()
{
    // Parse cup.toml file.
    auto toml_config = data::Deserializer<data::Default>::deserialize(
        toml::parse_file((this->root / "cup.toml").string()));
    // Get all dependencies.

    CMakeContext context{
        .project_name = toml_config.project.name,
        .cmake_version = {3, 10},
        .current_dir = this->root,
        .root_dir = this->root,
    };
    // Generate cmake content.
    auto dependencies = get_all_dependencies(toml_config);
    std::vector<std::string> cmake_content;
    for (const auto &[name, info] : dependencies)
    {
        LOG_INFO("Generating cmake content for dependency ", name, " with type ", info.type);
        PluginLoader loader(info.type);
        context.project_name = name;
        context.current_dir = info.path;
        context.features = info.features;
        cmake_content.push_back(loader->gen_cmake(context, true));
    }
    auto end_name = toml_config.project.name;
    auto end_type = toml_config.project.type;
    LOG_INFO("Generating cmake content for dependency ", end_name, " with type ", end_type);
    PluginLoader loader(end_type);
    context.project_name = end_name;
    context.current_dir = this->root;
    cmake_content.push_back(loader->gen_cmake(context, false));
    // Write cmake content to file.
    {
        if (!fs::exists(Resource::build(this->root)))
            fs::create_directories(Resource::build(this->root));
        auto cmakelists = Resource::build(this->root) / "CMakeLists.txt";
        std::ofstream ofs(cmakelists);
        ofs << "# Generated by cup\n";
        auto [major, minor] = context.cmake_version;
        ofs << "cmake_minimum_required(VERSION " << major << "." << minor << ")\n";
        ofs << "project(" << context.project_name << ")\n";
        if (toml_config.build.has_value() &&
            toml_config.build->export_data.has_value() &&
            toml_config.build->export_data->compile_commands.has_value())
        {
            ofs << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
        }
        for (const auto &content : cmake_content)
            ofs << content << "\n";
    }

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
        cmake.build(Resource::build(this->root) / "cmake");
        cmake.config(this->is_release);
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
            LOG_WARN("Generator \"", toml_config.build->generator.value_or(
#ifdef _WIN32
                                         "Visual Studio 17 2022"
#else
                                         "Unix Makefiles"
#endif
                                         ),
                     "\" does not support compile_commands.json.");
        }
        if (!compile_commands.empty() && fs::exists(from))
        {
            if (compile_commands.is_relative())
                compile_commands = this->root / compile_commands;
            auto to = compile_commands / "compile_commands.json";
            fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        }
    }
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
            "installed",
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
    if (fs::exists(build_dir))
        fs::remove_all(build_dir);
    return 0;
}
