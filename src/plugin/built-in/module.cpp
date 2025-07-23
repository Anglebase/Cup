#ifdef FORVSCODE
#include "module.h"
#endif

#include "plugin/built-in/module.h"
#include "plugin/built-in/utils.h"
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

Result<std::string, std::string> ModulePlugin::getName() const
{
    using namespace std::string_literals;
    return Ok<std::string>("module"s);
}

Result<int, std::string> ModulePlugin::run_new(const NewData &data)
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
    return Ok<std::string>(0);
}

Result<std::string, std::string> ModulePlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency)
{
    if (is_dependency)
        throw std::runtime_error("Module project cannot be used as a dependency.");
    auto [name, _1, current_dir, root_dir, _2, dependencies] = ctx;
    auto src = current_dir / "src";
    auto config = data::parse_toml_file<data::Module>(root_dir / "cup.toml");

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
    // Target specific configuration items
    std::vector<std::string> for_target;
    if (config.target)
    {
        for (const auto &[target, cfg] : *config.target)
        {
            std::unordered_map<std::string, std::string> replacements = {{"TARGET", '"' + target + '"'}};
            {
                auto extend = gen_map("TARGET_", std::optional(cfg));
                replacements.insert(extend.begin(), extend.end());
            }
            {
                auto extend = gen_map("TARGET_DEBUG_", cfg.debug);
                replacements.insert(extend.begin(), extend.end());
            }
            {
                auto extend = gen_map("TARGET_RELEASE_", cfg.release);
                replacements.insert(extend.begin(), extend.end());
            }
            FileTemplate temp{
#include "template/cmake/target.cmake"
                ,
                replacements};
            for_target.push_back(temp.getContent());
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
    return Ok<std::string>(FileTemplate{
#include "template/module/module.cmake"
        ,
        {
            {"FOR_GEN", join(for_gen, "\n")},
            {"FOR_MODE", join(for_mode, "\n")},
            {"FOR_TEST", join(for_tests, "\n")},
            {"FOR_FEAT", join(for_feats, "\n")},
            {"FOR_TARGET", join(for_target, "\n")},
            {"SOURCES", join(this->get_all_source_files(current_dir), " ", dealpath)},
            {"OUT_NAME", name},
            {"OUT_DIR", dealpath(Resource::mod(root_dir))},
            {"UNIQUE", name + "_" + replace(config.project.version, ".", "_")},
            {"TEST_MAIN_FILES", join(this->get_test_main_files(current_dir), " ", dealpath)},
            {"TEST_OUT_DIR", dealpath(Resource::bin(root_dir) / "tests")},
            {"DEPS", join(deps, " ")},
            {"INC", dealpath(current_dir / "include")},
            {"STDC", config.build && config.build->stdc ? std::to_string(*config.build->stdc) : ""},
            {"STDCXX", config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : ""},
        },
    }
                               .getContent());
}

Result<fs::path, std::string> ModulePlugin::run_project(const RunProjectData &data)
{
    auto [command, root, name, is_debug] = data;
    if (!command)
    {
        using namespace std::string_literals;
        return Err<fs::path>("No target specified."s);
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
        return Err<fs::path>("Cannot find target: " + result.filename().string());

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

    return Ok<std::string>(result);
}

Result<std::optional<std::string>, std::string> ModulePlugin::get_target(const RunProjectData &data) const
{
    auto [command, root, name, is_debug] = data;
    if (!command)
        return Ok<std::string>(std::optional<std::string>());
    auto config = data::parse_toml_file<data::Module>(root / "cup.toml");
    auto unique_suffix = name + "_" + replace(config.project.version, ".", "_");
    auto filename = split(fs::path(*command).filename().string(), ".")[0];
    return Ok<std::string>(std::optional("test_" + filename + "_" + unique_suffix));
}

Result<int, std::string> ModulePlugin::show_help(const cmd::Args &command) const
{
    std::cout <<
#include "template/help/built-in/module.txt"
        ;
    return Ok<std::string>(0);
}
