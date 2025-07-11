#ifdef FORVSCODE
#include "interface.h"
#endif

#include "plugin/built-in/interface.h"
#include "plugin/built-in/utils.h"
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

std::string InterfacePlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &except)
{
    auto [name, _1, current_dir, root_dir, features, dependencies] = ctx;
    auto config = data::parse_toml_file<data::Interface>(current_dir / "cup.toml");
    std::vector<std::string> feats;
    if (is_dependency)
        feats = get_features(ctx.features, config.features);
    else if (config.build)
        feats = get_features(config.build->features, config.features);
    std::vector<std::string> deps;
    {
        for (const auto &dep : dependencies)
            deps.push_back(dep);
    }
    std::vector<std::string> for_feats;
    if (config.feature)
    {
        std::vector<std::string> has_unique;
        for (const auto &[name, cfg] : *config.feature)
        {
            if (std::find(feats.begin(), feats.end(), name) == feats.end())
                continue;
            has_unique.push_back(name);
            std::unordered_map<std::string, std::string> replacements = {{"UNIQUE", name}};
            {
                auto extend = gen_map("FEAT_", std::optional(cfg));
                replacements.insert(extend.begin(), extend.end());
            }
            {
                auto extend = gen_map("FEAT_DEBUG_", cfg.debug);
                replacements.insert(extend.begin(), extend.end());
            }
            {
                auto extend = gen_map("FEAT_RELEASE_", cfg.release);
                replacements.insert(extend.begin(), extend.end());
            }
            FileTemplate temp{
#include "template/cmake/feat.cmake"
                ,
                replacements};
            for_feats.push_back(temp.getContent());
        }
        FileTemplate temp{
#include "template/cmake/feature.cmake"
            ,
            gen_feat_replacement(has_unique)};
        for_feats.push_back(temp.getContent());
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
#include "template/cmake/gen.cmake"
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
#include "template/cmake/mode.cmake"
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
#include "template/cmake/tests.cmake"
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
#include "template/cmake/examples.cmake"
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
            {"FOR_FEAT", join(for_feats, "\n")},
            {"FOR_EXAMPLES", join(for_examples, "\n")},
            {"EXPORT_NAME", name},
            {"IS_DEP", is_dependency ? "ON" : "OFF"},
            {"DEPS", join(deps, " ")},
            {"UNIQUE", name + "_" + replace(config.project.version, ".", "_")},
            {"TEST_MAIN_FILES", join(this->get_all_tests_main_files(current_dir), " ", dealpath)},
            {"TEST_OUT_DIR", dealpath(Resource::bin(root_dir) / "tests")},
            {"EXAMPLE_MAIN_FILES", join(this->get_examples_main_files(current_dir), " ", dealpath)},
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

    auto mode = result.parent_path().filename().string();
    auto dll = mode == "Debug" || mode == "Release" ? Resource::dll(root) / mode : Resource::dll(root);
    if (fs::exists(dll))
        for (const auto &entry : fs::directory_iterator(dll))
        {
            auto dll_file = entry.path();
            auto link = result.parent_path() / dll_file.filename();
            if (fs::exists(link))
                fs::remove(link);
            fs::create_hard_link(dll_file, link);
        }

    return result;
}

std::optional<std::string> InterfacePlugin::get_target(const RunProjectData &data, std::optional<std::string> &except) const
{
    auto [command, root, name, is_debug] = data;
    if (!command)
        return std::optional<std::string>();
    auto config = data::parse_toml_file<data::Interface>(root / "cup.toml");
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
