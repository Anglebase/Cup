#ifdef FORVSCODE
#include "module.h"
#endif

#include "plugin/built-in/module.h"
#include "template.h"
#include "res.h"
#include "utils/utils.h"
#include "toml/default/module.h"
#include <fstream>

std::string ModulePlugin::getName(std::optional<std::string> &except) const
{
    return "module";
}

int ModulePlugin::run_new(const NewData &data, std::optional<std::string> &except)
{
    auto [name, type, root] = data;
    auto project = root / name;
    auto src = project / "src";
    fs::create_directories(src);
    {
        auto demo_file = src / (name + ".cpp");
        std::ofstream ofs(demo_file);
        ofs << FileTemplate{
#include "template/module/module.cpp.txt"
            ,
            {{"NAME", name}}}
                   .getContent();
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
    {
        auto gitignore = project / ".gitignore";
        std::ofstream ofs(gitignore);
        ofs << "/target" << std::endl;
    }
    return 0;
}

std::string ModulePlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except)
{
    auto [name, _1, current_dir, root_dir, _2] = ctx;
    auto src = current_dir / "src";
    auto config = data::Deserializer<data::Module>::deserialize(toml::parse_file((current_dir / "cup.toml").string()));
    if (is_dependency)
    {
        except = "Module project cannot be used as a dependency.";
        return std::string();
    }
    return FileTemplate{
#include "template/module/module.cmake"
        ,
        {
            {
                "OUT_NAME",
                name,
            },
            {
                "SOURCES",
                [&]
                {
                    std::vector<fs::path> sources;
                    for (const auto &entry : fs::recursive_directory_iterator(src))
                        if (entry.is_regular_file())
                            sources.push_back(entry.path());
                    return join(sources, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "INCLUDE_DIR",

                [&]
                {
                    std::vector<fs::path> includes{current_dir / "include"};
                    if (config.build && config.build->includes)
                        for (const auto &include : *config.build->includes)
                            includes.push_back(include);
                    return join(includes, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "DEPENDS",
                config.dependencies
                    ? join(*config.dependencies, " ", [](const auto &dep)
                           { return dep.first; })
                    : "",
            },
            {
                "DEFINES",
                config.build && config.build->defines
                    ? join(*config.build->defines, " ", [](const std::string &define)
                           { return "-D" + define; })
                    : "",
            },
            {
                "COPTIONS",
                config.build && config.build->compiler_options
                    ? join(*config.build->compiler_options, " ")
                    : "",
            },
            {
                "LOPTIONS",
                config.build && config.build->link_options
                    ? join(*config.build->link_options, " ")
                    : "",
            },
            {
                "LINK_DIRS",
                config.build && config.build->link_dirs
                    ? join(*config.build->link_dirs, " ", [](const fs::path &dir)
                           { return '"' + replace(dir.string()) + '"'; })
                    : "",
            },
            {
                "LINK_LIBS",
                config.build && config.build->link_libs
                    ? join(*config.build->link_libs, " ")
                    : "",
            },
            {
                "TEST_FILES",
                [&]
                {
                    if (!fs::exists(src / "tests"))
                        return std::string();
                    std::vector<fs::path> tests;
                    for (const auto &entry : fs::directory_iterator(src / "tests"))
                        if (entry.is_regular_file())
                            tests.push_back(entry.path());
                    return join(tests, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "TEST_OUTDIR",
                '"' + replace((Resource::bin(root_dir) / "tests").string()) + '"',
            },
            {
                "TEST_DEFINES",
                config.tests && config.tests->defines
                    ? join(*config.tests->defines, " ", [](const std::string &define)
                           { return "-D" + define; })
                    : "",
            },
            {
                "TEST_INCLUDES",
                config.tests && config.tests->includes
                    ? join(*config.tests->includes, " ", [](const fs::path &include)
                           { return '"' + replace(include.string()) + '"'; })
                    : "",
            },
            {
                "OUT_DIR",
                '"' + replace(Resource::mod(root_dir).string()) + '"',
            },
            {
                "UNIQUE_SUFFIX",
                name + "_" + replace(config.project.version, ".", "_"),
            },
            {
                "STDC",
                config.build && config.build->stdc ? std::to_string(*config.build->stdc) : "",
            },
            {
                "STDCXX",
                config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : "",
            },
        }}
        .getContent();
}

fs::path ModulePlugin::run_project(const RunProjectData &data, std::optional<std::string> &except)
{
    auto [command, root, name, is_debug] = data;
    if (!command)
    {
        except = "No target specified.";
        return fs::path();
    }
    auto result = Resource::bin(root) / "tests" / *command;
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

std::optional<std::string> ModulePlugin::get_target(const RunProjectData &data, std::optional<std::string> &except) const
{
    auto [command, root, name, is_debug] = data;
    if (!command)
        return std::optional<std::string>();
    auto config = data::Deserializer<data::Module>::deserialize(toml::parse_file((root / "cup.toml").string()));
    auto unique_suffix = name + "_" + replace(config.project.version, ".", "_");
    auto filename = split(fs::path(*command).filename().string(), ".")[0];
    return "test_" + filename + "_" + unique_suffix;
}

int ModulePlugin::show_help(const cmd::Args &command, std::optional<std::string> &except) const
{
    std::cout <<
#include "template/help/built-in/module.txt"
        ;
    return 0;
}
