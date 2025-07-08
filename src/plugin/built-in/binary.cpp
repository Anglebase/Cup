#ifdef FORVSCODE
#include "binary.h"
#endif

#include "plugin/built-in/binary.h"
#include "template.h"
#include "utils/utils.h"
#include "toml/default/binary.h"
#include "res.h"
#include <fstream>
#include <algorithm>

void BinaryPlugin::_get_source_files(const fs::path &dir, std::vector<fs::path> &files)
{
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_directory() && entry.path().stem() != "bin")
            this->_get_source_files(entry.path(), files);
        else if (entry.is_regular_file() && entry.path().stem() != "main")
            files.push_back(entry.path());
    }
}

std::vector<fs::path> BinaryPlugin::get_source_files(const fs::path &root)
{
    std::vector<fs::path> source_files;
    if (!fs::exists(root / "src"))
        throw std::runtime_error("Cannot find required directory 'src' in project root directory.");
    this->_get_source_files(root / "src", source_files);
    return source_files;
}

fs::path BinaryPlugin::get_main_file(const fs::path &root)
{
    if (!fs::exists(root / "src"))
        throw std::runtime_error("Cannot find required directory 'src' in project root directory.");
    auto src = root / "src";
    std::optional<fs::path> main_file;
    for (const auto &entry : fs::directory_iterator(src))
    {
        if (entry.is_regular_file() && entry.path().stem() == "main")
        {
            main_file = entry.path();
            break;
        }
    }
    if (!main_file)
        throw std::runtime_error("Cannot find required file 'main' in 'src' directory.");
    return *main_file;
}

std::vector<fs::path> BinaryPlugin::get_bin_main_files(const fs::path &root)
{
    if (!fs::exists(root / "src" / "bin"))
        return std::vector<fs::path>();
    std::vector<fs::path> bin_main_files;
    for (const auto &entry : fs::directory_iterator(root / "src" / "bin"))
    {
        if (entry.is_regular_file())
            bin_main_files.push_back(entry.path());
    }
    return bin_main_files;
}

std::vector<fs::path> BinaryPlugin::get_tests_main_files(const fs::path &root)
{
    if (!fs::exists(root / "tests"))
        return std::vector<fs::path>();
    std::vector<fs::path> tests_main_files;
    for (const auto &entry : fs::directory_iterator(root / "tests"))
    {
        if (entry.is_regular_file())
            tests_main_files.push_back(entry.path());
    }
    return tests_main_files;
}

inline std::string dealpath(const fs::path &p)
{
    return '"' + replace(p.string()) + '"';
}

std::string BinaryPlugin::getName(std::optional<std::string> &) const { return "binary"; }

int BinaryPlugin::run_new(const NewData &data, std::optional<std::string> &)
{
    auto [name, type, root] = data;
    auto project = root / name;
    {
        auto src = project / "src";
        if (!fs::exists(src))
            fs::create_directories(src);
        auto main_file = src / "main.cpp";
        std::ofstream ofs(main_file);
        ofs <<
#include "template/binary/main.cpp.txt"
            ;
    }
    {
        auto gitignore = project / ".gitignore";
        std::ofstream ofs(gitignore);
        ofs << "/target" << std::endl;
    }
    {
        auto cup_toml = project / "cup.toml";
        std::ofstream ofs(cup_toml);
        ofs << FileTemplate{
#include "template/cup.toml.txt"
            ,
            {
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

std::string BinaryPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except)
{
    if (is_dependency)
        throw std::runtime_error("'binary' project cannot be a dependency.");
    auto [name, _1, current_dir, root_dir, _2] = ctx;
    auto project = current_dir;
    auto src = project / "src";
    auto include = project / "include";
    auto config = data::parse_toml_file<data::Binary>(project / "cup.toml");
    std::vector<std::string> deps;
    {
        std::vector<std::string> feats;
        if (is_dependency)
            feats = get_features(ctx.features, config.features);
        else if (config.build)
            feats = get_features(config.build->features, config.features);
        if (config.dependencies)
        {
            for (const auto &[name, info] : *config.dependencies)
            {
                if (!info.optional ||
                    std::find_if(
                        info.optional->begin(), info.optional->end(),
                        [&](const std::string &f)
                        {
                            return std::find(feats.begin(), feats.end(), f) != feats.end();
                        }) != info.optional->end())
                    deps.push_back(name);
            }
        }
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
#include "template/binary/gen.cmake"
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
#include "template/binary/mode.cmake"
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
#include "template/binary/tests.cmake"
            ,
            replacements};
        for_tests.push_back(temp.getContent());
    }
    return FileTemplate{
#include "template/binary/binary.cmake"
        ,
        {
            {"FOR_GEN", join(for_gen, "\n")},
            {"FOR_MODE", join(for_mode, "\n")},
            {"FOR_TEST", join(for_tests, "\n")},
            {"MAIN_FILE", dealpath(this->get_main_file(current_dir))},
            {"BIN_MAIN_FILES", join(this->get_bin_main_files(current_dir), " ", dealpath)},
            {"SOURCES", join(this->get_source_files(current_dir), " ", dealpath)},
            {"OUT_NAME", name},
            {"OUT_DIR", dealpath(Resource::bin(root_dir))},
            {"UNIQUE", name + "_" + replace(config.project.version, ".", "_")},
            {"TEST_MAIN_FILES", join(this->get_tests_main_files(current_dir), " ", dealpath)},
            {"TEST_OUT_DIR", dealpath(Resource::bin(root_dir) / "tests")},
            {"DEPS", join(deps, " ")},
            {"INC", dealpath(current_dir / "include")},
            {"STDC", config.build && config.build->stdc ? std::to_string(*config.build->stdc) : ""},
            {"STDCXX", config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : ""},
        },
    }
        .getContent();
}

fs::path BinaryPlugin::run_project(const RunProjectData &data, std::optional<std::string> &except)
{
    auto [command, root, name, is_debug] = data;
    fs::path result = Resource::bin(root) / (command && command != "main" ? *command : name);
#ifdef _WIN32
    result.replace_extension(".exe");
#else
    result.replace_extension();
#endif
    if (!fs::exists(result))
        result = result.parent_path() / (is_debug ? "Debug" : "Release") / result.filename();
    if (!fs::exists(result))
        except = "Cannot find executable file" + result.filename().string() + ".";
    return result;
}

std::optional<std::string> BinaryPlugin::get_target(const RunProjectData &data, std::optional<std::string> &except) const
{
    auto config = data::parse_toml_file<data::Binary>(data.root / "cup.toml");
    const auto unique_suffix = replace(config.project.version, ".", "_");
    const auto target_name = data.name;
    auto [command, root, name, is_debug] = data;
    if (!command)
        return std::optional<std::string>();
    auto target = *command;
    if (target == "main")
    {
        return target_name + "_" + unique_suffix;
    }
    else if (target.starts_with("tests/"))
    {
        auto str = target.substr(5);
        auto filename = split(str, ".")[0];
        return "test_" + target_name + "_" + filename + "_" + unique_suffix;
    }
    else if (target.starts_with("bin/"))
    {
        auto str = target.substr(4);
        auto filename = split(str, ".")[0];
        return "bin_" + target_name + "_" + filename + "_" + unique_suffix;
    }
    else
    {
        except = "Invalid target: " + target;
    }
    return std::nullopt;
}

int BinaryPlugin::show_help(const cmd::Args &command, std::optional<std::string> &) const
{
    std::cout <<
#include "template/help/built-in/binary.txt"
        ;
    return 0;
}
