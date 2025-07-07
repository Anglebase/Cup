#ifdef FORVSCODE
#include "static.h"
#endif

#include "plugin/built-in/static.h"
#include "template.h"
#include "utils/utils.h"
#include "toml/default/static.h"
#include "res.h"
#include <fstream>
void StaticPlugin::_get_source_files(const fs::path &dir, std::vector<fs::path> &src_files) const
{
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_directory())
        {
            _get_source_files(entry.path(), src_files);
        }
        else if (entry.is_regular_file())
        {
            src_files.push_back(entry.path());
        }
    }
}

std::vector<fs::path> StaticPlugin::get_source_files(const fs::path &root) const
{
    if (!fs::exists(root / "src"))
        throw std::runtime_error("Cannot find required directory 'src' in project root directory.");
    std::vector<fs::path> source_files;
    this->_get_source_files(root / "src", source_files);
    return source_files;
}

std::vector<fs::path> StaticPlugin::get_test_mains(const fs::path &root) const
{
    if (!fs::exists(root / "tests"))
        return std::vector<fs::path>();
    std::vector<fs::path> test_mains;
    for (const auto &entry : fs::directory_iterator(root / "tests"))
    {
        if (entry.is_regular_file())
            test_mains.push_back(entry.path());
    }
    return test_mains;
}

std::vector<fs::path> StaticPlugin::get_example_mains(const fs::path &root) const
{
    if (!fs::exists(root / "examples"))
        return std::vector<fs::path>();
    std::vector<fs::path> example_mains;
    for (const auto &entry : fs::directory_iterator(root / "examples"))
    {
        if (entry.is_regular_file())
            example_mains.push_back(entry.path());
    }
    return example_mains;
}
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
std::string StaticPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except)
{
    auto [name, _1, current_dir, root_dir, features] = ctx;
    auto src = current_dir / "src";
    auto config = data::Deserializer<data::Static>::deserialize(toml::parse(read_file(current_dir / "cup.toml")));

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
#include "template/static/gen.cmake"
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
#include "template/static/mode.cmake"
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
#include "template/static/tests.cmake"
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
#include "template/static/examples.cmake"
            ,
            replacements};
        for_examples.push_back(temp.getContent());
    }
    return FileTemplate{
#include "template/static/static.cmake"
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
            {"TEST_MAIN_FILES", join(this->get_test_mains(root_dir), " ", dealpath)},
            {"TEST_OUT_DIR", dealpath(Resource::bin(root_dir) / "tests")},
            {"EXAMPLE_MAIN_FILES", join(this->get_example_mains(root_dir), " ", dealpath)},
            {"EXAMPLE_OUT_DIR", dealpath(Resource::bin(root_dir) / "examples")},
            {"INC", dealpath(current_dir / "include")},
            {"EXPORT_INC", dealpath(current_dir / "export")},
            {"SOURCES", join(this->get_source_files(root_dir), " ", dealpath)},
            {"STDC", config.build && config.build->stdc ? std::to_string(*config.build->stdc) : ""},
            {"STDCXX", config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : ""},
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
