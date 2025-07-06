#ifdef FORVSCODE
#include "static.h"
#endif

#include "plugin/built-in/static.h"
#include "template.h"
#include "utils/utils.h"
#include "toml/default/static.h"
#include "res.h"
#include <fstream>

std::string StaticPlugin::getName(std::optional<std::string> &except) const
{
    return "static";
}

int StaticPlugin::run_new(const NewData &data, std::optional<std::string> &except)
{
    auto [name, type, root] = data;
    auto project = root / name;
    auto src = project / "src";
    auto export_dir = project / "export" / name;
    {
        auto src_file = src / (name + ".cpp");
        if (!fs::exists(src))
            fs::create_directories(src);
        std::ofstream ofs(src_file);
        ofs << FileTemplate{
#include "template/static/static.cpp.txt"
            ,
            {{"NAME", name}}}
                   .getContent();
    }
    {
        auto export_file = export_dir / (name + ".h");
        if (!fs::exists(export_dir))
            fs::create_directories(export_dir);
        std::ofstream ofs(export_file);
        ofs << FileTemplate{
#include "template/static/export.h.txt"
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

std::string StaticPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except)
{
    auto [name, _1, current_dir, root_dir, features] = ctx;
    auto src = current_dir / "src";
    auto config = data::Deserializer<data::Static>::deserialize(toml::parse(read_file(current_dir / "cup.toml")));
    return FileTemplate{
#include "template/static/static.cmake"
        ,
        {
            {
                "EXPORT_NAME",
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
                '"' + replace((current_dir / "include").string()) + '"',
            },
            {
                "EXPORT_DIR",
                [&]
                {
                    std::vector<fs::path> export_dirs{(current_dir / "export").string()};
                    if (config.build && config.build->includes)
                        for (const auto &include_dir : *config.build->includes)
                            export_dirs.push_back(include_dir);
                    return join(export_dirs, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "DEFINES",
                config.build && config.build->defines
                    ? join(*config.build->defines, " ", [](const std::string &d)
                           { return "-D" + d; }) +
                          " " +
                          join(features, " ", [](const std::string &f)
                               { return "-D" + f; })
                    : "",
            },
            {
                "LINK_DIRS",
                config.build && config.build->link_dirs
                    ? join(*config.build->link_dirs, " ", [](const fs::path &d)
                           { return '"' + replace(d.string()) + '"'; })
                    : "",
            },
            {
                "LINK_LIBS",
                config.build && config.build->link_libs
                    ? join(*config.build->link_libs, " ")
                    : "",
            },
            {
                "STDC",
                config.build && config.build->stdc ? std::to_string(*config.build->stdc) : "",
            },
            {
                "STDCXX",
                config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : "",
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
                "OUT_NAME",
                name,
            },
            {
                "OUT_DIR",
                '"' + replace(Resource::lib(root_dir).string()) + '"',
            },
            {
                "IS_DEP",
                is_dependency ? "ON" : "OFF",
            },
            {
                "TEST_MAIN_FILES",
                [&]
                {
                    if (!fs::exists(current_dir / "tests"))
                        return std::string();
                    std::vector<fs::path> test_main_files;
                    for (const auto &entry : fs::directory_iterator(current_dir / "tests"))
                        if (entry.is_regular_file())
                            test_main_files.push_back(entry.path());
                    return join(test_main_files, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "EXAMPLE_MAIN_FILES",
                [&]
                {
                    if (!fs::exists(current_dir / "examples"))
                        return std::string();
                    std::vector<fs::path> example_main_files;
                    for (const auto &entry : fs::directory_iterator(current_dir / "examples"))
                        if (entry.is_regular_file())
                            example_main_files.push_back(entry.path());
                    return join(example_main_files, " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "UNIQUE_SUFFIX",
                name + "_" + replace(config.project.version, ".", "_"),
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
                "TEST_DEFINES",
                config.tests && config.tests->defines
                    ? join(*config.tests->defines, " ", [](const std::string &d)
                           { return "-D" + d; })
                    : "",
            },
            {
                "TEST_INC",
                config.tests && config.tests->includes
                    ? join(*config.tests->includes, " ", [](const fs::path &d)
                           { return '"' + replace(d.string()) + '"'; })
                    : "",
            },
            {
                "EXAMPLE_INC",
                config.examples && config.examples->includes
                    ? join(*config.examples->includes, " ", [](const fs::path &d)
                           { return '"' + replace(d.string()) + '"'; })
                    : "",
            },
            {
                "EXAMPLE_DEFINES",
                config.examples && config.examples->defines
                    ? join(*config.examples->defines, " ", [](const std::string &d)
                           { return "-D" + d; })
                    : "",
            },
            {
                "DEPENDS",
                config.dependencies
                    ? join(*config.dependencies, " ",
                           [](const std::pair<std::string, data::Dependency> &p)
                           { return p.first; })
                    : "",
            },
        },
    }
        .getContent();
}

fs::path StaticPlugin::run_project(const RunProjectData &data, std::optional<std::string> &except)
{
    auto [command, root, name, is_debug] = data;
    if (!command)
    {
        except = "No command specified";
        return fs::path();
    }
    auto result = Resource::bin(root) / *command;
#ifdef _WIN32
    result = result.replace_extension(".exe");
#else
    result = result.replace_extension();
#endif
    if (!fs::exists(result))
        result = result.parent_path() / (is_debug ? "Debug" : "Release") / result.filename();
    if (!fs::exists(result))
        except = "Cannot find executable file" + result.filename().string() + ".";
    return result;
}

std::optional<std::string> StaticPlugin::get_target(const RunProjectData &data, std::optional<std::string> &except) const
{
    auto [command, root, name, is_debug] = data;
    auto config = data::Deserializer<data::Static>::deserialize(toml::parse(read_file(root / "cup.toml")));
    auto unique_suffix = name + "_" + replace(config.project.version, ".", "_");
    if (!command)
        return std::optional<std::string>();
    auto target = *command;
    if (target.starts_with("tests/"))
    {
        auto str = target.substr(6);
        auto filename = split(str, ".")[0];
        return "test_" + filename + '_' + unique_suffix;
    }
    else if (target.starts_with("examples/"))
    {
        auto str = target.substr(9);
        auto filename = split(str, ".")[0];
        return "example_" + filename + '_' + unique_suffix;
    }
    else
    {
        except = "Invalid target: " + target;
    }
    return std::nullopt;
}

int StaticPlugin::show_help(const cmd::Args &command, std::optional<std::string> &except) const
{
    std::cout <<
#include "template/help/built-in/static.txt"
        ;
    return 0;
}
