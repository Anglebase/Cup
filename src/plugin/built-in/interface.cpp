#ifdef FORVSCODE
#include "interface.h"
#endif

#include "plugin/built-in/interface.h"
#include "template.h"
#include "res.h"
#include "toml/default/interface.h"
#include "utils/utils.h"
#include <fstream>

std::vector<fs::path> InterfacePlugin::get_all_tests_main_files(const fs::path &root)
{
    if (!fs::exists(root / "tests"))
        return {};
    std::vector<fs::path> result;
    for (const auto &entry : fs::directory_iterator(root / "tests"))
    {
        if (entry.is_regular_file())
            result.push_back(entry.path());
    }
    return result;
}

std::vector<fs::path> InterfacePlugin::get_examples_main_files(const fs::path &root)
{
    if (!fs::exists(root / "examples"))
        return {};
    std::vector<fs::path> result;
    for (const auto &entry : fs::directory_iterator(root / "examples"))
    {
        if (entry.is_regular_file())
            result.push_back(entry.path());
    }
    return result;
}

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

template <class T>
std::unordered_map<std::string, std::string> gen_map(const std::string &prefix, const std::optional<T> &config)
{
    return {
        {
            prefix + "INCLUDE_DIRS",
            config && config->includes
                ? join(*config->includes, " ", [](const fs::path &p)
                       { return '"' + replace(p.string()) + '"'; })
                : "",
        },
        {
            prefix + "LIB_DIRS",
            config && config->link_dirs
                ? join(*config->link_dirs, " ", [](const fs::path &p)
                       { return '"' + replace(p.string()) + '"'; })
                : "",
        },
        {
            prefix + "LIBS",
            config && config->link_libs ? join(*config->link_libs, " ") : "",
        },
        {
            prefix + "DEFINES",
            config && config->defines
                ? join(*config->defines, " ", [](const std::string &s)
                       { return "-D" + s; })
                : "",
        },
        {
            prefix + "COPTIONS",
            config && config->compile_options ? join(*config->compile_options, " ") : "",
        },
        {
            prefix + "LINKOPTIONS",
            config && config->link_options ? join(*config->link_options, " ") : "",
        },
    };
}

inline std::string dealpath(const fs::path &p)
{
    return '"' + replace(p.string()) + '"';
}
std::string InterfacePlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except)
{
    auto [name, _, current_dir, root_dir, features] = ctx;
    auto config = data::Deserializer<data::Interface>::deserialize(toml::parse_file((current_dir / "cup.toml").string()));

    // Generator specific configuration items
    std::vector<std::string> for_gen;
    if (config.generator)
    {
        for (const auto &[gen, cfg] : *config.generator)
        {
            std::unordered_map<std::string, std::string> replacements = {{"GEN", '"' + gen + '"'}};
            {
                auto extend = gen_map("GEN_", std::optional(cfg));
                replacements.insert(extend.begin(), extend.end());
            }
            {
                auto extend = gen_map("GEN_DEBUG_", cfg.debug);
                replacements.insert(extend.begin(), extend.end());
            }
            {
                auto extend = gen_map("GEN_RELEASE_", cfg.release);
                replacements.insert(extend.begin(), extend.end());
            }
            FileTemplate temp{
#include "template/interface/gen.cmake"
                ,
                replacements};
            for_gen.push_back(temp.getContent());
        }
    }
    // Mode specific configuration items
    std::vector<std::string> for_mode;
    {
        std::unordered_map<std::string, std::string> replacements;
        {
            auto extend = gen_map("MODE_", config.build ? config.build : std::nullopt);
            replacements.insert(extend.begin(), extend.end());
        }
        {
            auto extand = gen_map("MODE_DEBUG_", config.build ? config.build->debug : std::nullopt);
            replacements.insert(extand.begin(), extand.end());
        }
        {
            auto extand = gen_map("MODE_RELEASE_", config.build ? config.build->release : std::nullopt);
            replacements.insert(extand.begin(), extand.end());
        }
        FileTemplate temp{
#include "template/interface/mode.cmake"
            ,
            replacements};
        for_mode.push_back(temp.getContent());
    }
    // Tests mode specific configuration items
    std::vector<std::string> for_tests;
    {
        std::unordered_map<std::string, std::string> replacements;
        {
            auto extend = gen_map("TEST_", config.tests ? config.tests : std::nullopt);
            replacements.insert(extend.begin(), extend.end());
        }
        {
            auto extand = gen_map("TEST_DEBUG_", config.tests ? config.tests->debug : std::nullopt);
            replacements.insert(extand.begin(), extand.end());
        }
        {
            auto extand = gen_map("TEST_RELEASE_", config.tests ? config.tests->release : std::nullopt);
            replacements.insert(extand.begin(), extand.end());
        }
        FileTemplate temp{
#include "template/interface/tests.cmake"
            ,
            replacements};
        for_tests.push_back(temp.getContent());
    }
    // Examples mode specific configuration items
    std::vector<std::string> for_examples;
    {
        std::unordered_map<std::string, std::string> replacements;
        {
            auto extend = gen_map("EXAMPLE_", config.examples ? config.examples : std::nullopt);
            replacements.insert(extend.begin(), extend.end());
        }
        {
            auto extand = gen_map("EXAMPLE_DEBUG_", config.examples ? config.examples->debug : std::nullopt);
            replacements.insert(extand.begin(), extand.end());
        }
        {
            auto extand = gen_map("EXAMPLE_RELEASE_", config.examples ? config.examples->release : std::nullopt);
            replacements.insert(extand.begin(), extand.end());
        }
        FileTemplate temp{
#include "template/interface/examples.cmake"
            ,
            replacements};
        for_examples.push_back(temp.getContent());
    }
    return FileTemplate{
#include "template/interface/interface.cmake"
        ,
        {
            {"FOR_GEN", join(for_gen, "\n")},
            {"FOR_MODE", join(for_mode, "\n")},
            {"FOR_TESTS", join(for_tests, "\n")},
            {"FOR_EXAMPLES", join(for_examples, "\n")},
            {"EXPORT_NAME", name},
            {"IS_DEP", is_dependency ? "ON" : "OFF"},
            {"DEPS", config.dependencies ? join(*config.dependencies, " ", [](const std::pair<std::string, data::Dependency> &p)
                                                { return p.first; })
                                         : ""},
            {"UNIQUE", name + "_" + replace(config.project.version, ".", "_")},
            {"TEST_MAIN_FILES", join(this->get_all_tests_main_files(root_dir), " ", dealpath)},
            {"TEST_OUT_DIR", dealpath(Resource::bin(root_dir) / "tests")},
            {"EXAMPLE_MAIN_FILES", join(this->get_examples_main_files(root_dir), " ", dealpath)},
            {"EXAMPLE_OUT_DIR", dealpath(Resource::bin(root_dir) / "examples")},
            {"INC", dealpath(current_dir / "include")},
            {"STDC", config.build && config.build->stdc ? std::to_string(*config.build->stdc) : ""},
            {"STDCXX", config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : ""},
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
        auto str = command->substr(6);
        auto filename = split(fs::path(str).filename().string(), ".")[0];
        return "test_" + filename + "_" + unique_suffix;
    }
    else if (command->starts_with("examples/"))
    {
        auto str = command->substr(9);
        auto filename = split(fs::path(str).filename().string(), ".")[0];
        return "example_" + filename + "_" + unique_suffix;
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
