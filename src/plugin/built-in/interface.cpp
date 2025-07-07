#ifdef FORVSCODE
#include "interface.h"
#endif

#include "plugin/built-in/interface.h"
#include "template.h"
#include "res.h"
#include "toml/default/interface.h"
#include "utils/utils.h"
#include <fstream>

std::string InterfacePlugin::getName(std::optional<std::string> &except) const
{
    return "interface";
}

int InterfacePlugin::run_new(const NewData &data, std::optional<std::string> &except)
{
    auto [name, type, root] = data;
    auto project = root / name;
    auto include = project / "include";
    fs::create_directories(include);
    {
        auto export_path = include / (name + ".h");
        std::ofstream ofs(export_path);
        ofs << FileTemplate{
#include "template/interface/export.h.txt"
            ,
            {{"NAME", name}}}
                   .getContent();
    }
    {
        auto gitignore = project / ".gitignore";
        std::ofstream ofs(gitignore);
        ofs << "/target" << std::endl;
    }
    {
        auto config = project / "cup.toml";
        std::ofstream ofs(config);
        ofs << FileTemplate{
#include "template/cup.toml.txt"
            , {
                  {"NAME", name},
                  {"TYPE", type},
              }}.getContent();
    }
    return 0;
}

std::string InterfacePlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except)
{
    auto [name, _, current_dir, root_dir, features] = ctx;
    auto config = data::Deserializer<data::Interface>::deserialize(toml::parse_file((current_dir / "cup.toml").string()));
    return FileTemplate{
#include "template/interface/interface.cmake"
        ,
        {
            {
                "EXPORT_NAME",
                name,
            },
            {
                "UNIQUE_SUFFIX",
                name + '_' + replace(config.project.version, ".", "_"),
            },
            {
                "INCLUDE_DIR",
                [&]
                {
                    std::vector<fs::path> include_dirs{current_dir / "include"};
                    if (config.build && config.build->includes)
                        for (const auto &include : *config.build->includes)
                            include_dirs.push_back(include);
                    return join(include_dirs, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "LINK_DIRS",
                config.build && config.build->link_dirs
                    ? join(*config.build->link_dirs, " ", [](const fs::path &p)
                           { return '"' + replace(p.string()) + '"'; })
                    : "",
            },
            {
                "LINK_LIBS",
                config.build && config.build->link_libs
                    ? join(*config.build->link_libs, " ")
                    : "",
            },
            {
                "COPTIONS",
                config.build && config.build->compile_options
                    ? join(*config.build->compile_options, " ")
                    : "",
            },
            {
                "LOPTIONS",
                config.build && config.build->link_options
                    ? join(*config.build->link_options, " ")
                    : "",
            },
            {
                "DEFINES",
                config.build && config.build->defines
                    ? join(*config.build->defines, " ", [](const std::string &p)
                           { return "-D" + p; })
                    : "",
            },
            {
                "IS_DEP",
                is_dependency ? "ON" : "OFF",
            },
            {
                "TEST_FILES",
                [&]
                {
                    if (!fs::exists(current_dir / "tests"))
                        return std::string();
                    std::vector<fs::path> test_files;
                    for (const auto &entry : fs::directory_iterator(current_dir / "tests"))
                        if (entry.is_regular_file())
                            test_files.push_back(entry.path());
                    return join(test_files, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "TEST_DEFINES",
                config.tests && config.tests->defines
                    ? join(*config.tests->defines, " ", [](const std::string &p)
                           { return "-D" + p; })
                    : "",
            },
            {
                "TEST_INCLUDE",
                config.tests && config.tests->includes
                    ? join(*config.tests->includes, " ", [](const fs::path &p)
                           { return '"' + replace(p.string()) + '"'; })
                    : "",
            },
            {
                "EXAMPLE_FILES",
                [&]
                {
                    if (!fs::exists(current_dir / "examples"))
                        return std::string();
                    std::vector<fs::path> test_files;
                    for (const auto &entry : fs::directory_iterator(current_dir / "examples"))
                        if (entry.is_regular_file())
                            test_files.push_back(entry.path());
                    return join(test_files, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "EXAMPLE_DEFINES",
                config.examples && config.examples->defines
                    ? join(*config.examples->defines, " ", [](const std::string &p)
                           { return "-D" + p; })
                    : "",
            },
            {
                "EXAMPLE_INCLUDE",
                config.examples && config.examples->includes
                    ? join(*config.examples->includes, " ", [](const fs::path &p)
                           { return '"' + replace(p.string()) + '"'; })
                    : "",
            },
            {
                "TEST_OUT_DIR",
                '"' + replace((Resource::bin(root_dir) / "tests").string()) + '"',
            },
            {
                "EXAMPLE_OUT_DIR",
                '"' + replace((Resource::bin(root_dir) / "examples").string()) + '"',
            },
            {
                "DEPENDS",
                config.dependencies
                    ? join(*config.dependencies, " ", [](const std::pair<std::string, data::Dependency> &d)
                           { return d.first; })
                    : "",
            },
            {
                "STDC",
                config.build && config.build->stdc ? std::to_string(*config.build->stdc) : "",
            },
            {
                "STDCPP",
                config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : "",
            },
        },
    }
        .getContent();
}

fs::path InterfacePlugin::run_project(const RunProjectData &data, std::optional<std::string> &except)
{
    auto [command, root, name, is_debug] = data;
    if (!command)
    {
        except = "No target specified";
        return fs::path();
    }
    auto result = Resource::bin(root) / *command;
#ifdef _WIN32
    result.replace_extension(".exe");
#else
    result.replace_extension();
#endif
    if (!fs::exists(result))
        result = result.parent_path() / (is_debug ? "Debug" : "Release") / result.filename();
    if (!fs::exists(result))
        except = "Cannot find target: " + result.filename().string();
    return result;
}

std::optional<std::string> InterfacePlugin::get_target(const RunProjectData &data, std::optional<std::string> &except) const
{
    auto [command, root, name, is_debug] = data;
    if (!command)
        return std::optional<std::string>();
    auto config = data::Deserializer<data::Interface>::deserialize(toml::parse_file((root / "cup.toml").string()));
    auto unique_suffix = name + '_' + replace(config.project.version, ".", "_");
    if (command->starts_with("tests/"))
    {
        auto str = command->substr(5);
        auto filename = split(fs::path(str).filename().string(), ".")[0];
        return "test_" + name + "_" + filename + "_" + unique_suffix;
    }
    else if (command->starts_with("examples/"))
    {
        auto str = command->substr(8);
        auto filename = split(fs::path(str).filename().string(), ".")[0];
        return "example_" + name + "_" + filename + "_" + unique_suffix;
    }
    else
    {
        except = "Invalid target: " + *command;
        return std::optional<std::string>();
    }
}

int InterfacePlugin::show_help(const cmd::Args &command, std::optional<std::string> &except) const
{
    std::cout <<
#include "template/help/built-in/interface.txt"
        ;
    return 0;
}
