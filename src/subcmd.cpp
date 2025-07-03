#include "subcmd.h"
#include "plugin/loader.h"
#include "log.h"
#include "toml/default/default.h"
#include <algorithm>
#include "res.h"
#include "utils/utils.h"
#include <tuple>
#include "cmd/cmake.h"

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

fs::path get_path(const data::Dependency &dep, const std::string &version)
{
    if (dep.path)
        return dep.path.value();
    if (dep.url)
        throw std::runtime_error("Not implemented.");
    return fs::path();
}
using VersionInfo = std::tuple<int, int, int>;

VersionInfo parse_version(const std::string &version)
{
    auto parts = split(version, ".");
    if (parts.size() != 3)
        throw std::runtime_error("version '" + version + "' is not a valid version string.");
    return {std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2])};
}

struct DependencyInfo
{
    fs::path path;
    std::string type;
    VersionInfo version;
};

void _get_all_dependencies(const data::Default &toml_config, std::vector<std::pair<std::string, DependencyInfo>> &dependencies)
{
    for (const auto &[name, info] : toml_config.dependencies.value_or(std::map<std::string, data::Dependency>{}))
    {
        auto path = get_path(info, info.version);
        if (!fs::exists(path))
            throw std::runtime_error("Dependency '" + name + "' not found.");
        auto version = parse_version(info.version);
        auto dep_config = data::Deserializer<data::Default>::deserialize(
            toml::parse_file((path / "cup.toml").string()));
        auto dep_info = DependencyInfo{
            .path = path,
            .type = dep_config.project.type,
            .version = version,
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
    return 0;
}

List::List(const cmd::Args &args) : SubCommand(args) {}

int List::run()
{
    return 0;
}

Install::Install(const cmd::Args &args) : SubCommand(args) {}

int Install::run()
{
    return 0;
}

Uninstall::Uninstall(const cmd::Args &args) : SubCommand(args) {}

int Uninstall::run()
{
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
