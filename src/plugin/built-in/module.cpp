#ifdef FORVSCODE
#include "module.h"
#endif

#include "plugin/built-in/module.h"
#include "template.h"
#include "res.h"
#include "utils/utils.h"
#include "toml/default/module.h"
#include <fstream>

void ModulePlugin::_get_all_source_files(const fs::path &root, std::vector<fs::path> &src_files)
{
    for (const auto &entry : fs::directory_iterator(root))
    {
        if (entry.is_directory())
        {
            this->_get_all_source_files(entry.path(), src_files);
        }
        else if (entry.is_regular_file())
        {
            src_files.push_back(entry.path());
        }
    }
}

std::vector<fs::path> ModulePlugin::get_all_source_files(const fs::path &root)
{
    if (!fs::exists(root / "src"))
        throw std::runtime_error("Cannot find required directory 'src' in project root directory.");
    std::vector<fs::path> files;
    this->_get_all_source_files(root / "src", files);
    return files;
}

std::vector<fs::path> ModulePlugin::get_test_main_files(const fs::path &root)
{
    if (!fs::exists(root / "tests"))
        return std::vector<fs::path>();
    std::vector<fs::path> files;
    for (const auto &entry : fs::directory_iterator(root / "tests"))
    {
        if (entry.is_regular_file())
            files.push_back(entry.path());
    }
    return files;
}

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
#include "template/module/gen.cmake"
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
#include "template/module/mode.cmake"
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
#include "template/module/tests.cmake"
            ,
            replacements};
        for_tests.push_back(temp.getContent());
    }
    return FileTemplate{
#include "template/module/module.cmake"
        ,
        {
            {"FOR_GEN", join(for_gen, "\n")},
            {"FOR_MODE", join(for_mode, "\n")},
            {"FOR_TEST", join(for_tests, "\n")},
            {"SOURCES", join(this->get_all_source_files(current_dir), " ", dealpath)},
            {"OUT_NAME", name},
            {"OUT_DIR", dealpath(Resource::bin(root_dir))},
            {"UNIQUE", name + "_" + replace(config.project.version, ".", "_")},
            {"TEST_MAIN_FILES", join(this->get_test_main_files(current_dir), " ", dealpath)},
            {"TEST_OUT_DIR", dealpath(Resource::bin(root_dir) / "tests")},
            {"DEPS",
             config.dependencies
                 ? join(*config.dependencies, " ", [](const std::pair<std::string, data::Dependency> &d)
                        { return d.first; })
                 : ""},
            {"INC", dealpath(current_dir / "include")},
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
