#ifdef FORVSCODE
#include "shared.h"
#endif

#include "plugin/built-in/shared.h"
#include "plugin/built-in/utils.h"
#include "template.h"
#include "utils/utils.h"
#include "toml/default/shared.h"
#include "res.h"
#include <fstream>

void SharedPlugin::_get_source_files(const fs::path &dir, std::vector<fs::path> &src_files) const
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

std::vector<fs::path> SharedPlugin::get_source_files(const fs::path &root) const
{
    if (!fs::exists(root / "src"))
        throw std::runtime_error("Cannot find required directory 'src' in project root directory.");
    std::vector<fs::path> source_files;
    this->_get_source_files(root / "src", source_files);
    return source_files;
}

std::vector<fs::path> SharedPlugin::get_test_mains(const fs::path &root) const
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

std::vector<fs::path> SharedPlugin::get_example_mains(const fs::path &root) const
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

Result<std::string, std::string> SharedPlugin::getName() const
{
    using namespace std::string_literals;
    return Ok<std::string>("shared"s);
}

Result<int, std::string> SharedPlugin::run_new(const NewData &data)
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
#include "template/shared/shared.cpp.txt"
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
#include "template/shared/export.h.txt"
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

Result<std::string, std::string> SharedPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency)
{
    auto [name, _1, current_dir, root_dir, features, dependencies] = ctx;
    auto src = current_dir / "src";
    auto config = data::parse_toml_file<data::Shared>(current_dir / "cup.toml");

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
    return Ok<std::string>(FileTemplate{
#include "template/shared/shared.cmake"
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
            {"TEST_MAIN_FILES", join(this->get_test_mains(current_dir), " ", dealpath)},
            {"TEST_OUT_DIR", dealpath(Resource::bin(root_dir) / "tests")},
            {"EXAMPLE_MAIN_FILES", join(this->get_example_mains(current_dir), " ", dealpath)},
            {"EXAMPLE_OUT_DIR", dealpath(Resource::bin(root_dir) / "examples")},
            {"INC", dealpath(current_dir / "include")},
            {"EXPORT_INC", dealpath(current_dir / "export")},
            {"SOURCES", join(this->get_source_files(current_dir), " ", dealpath)},
            {"STDC", config.build && config.build->stdc ? std::to_string(*config.build->stdc) : ""},
            {"STDCXX", config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : ""},
            {"LIB_OUT_DIR", dealpath(Resource::lib(root_dir))},
            {"DLL_OUT_DIR", dealpath(Resource::dll(root_dir))},
        },
    }
        .getContent());
}

Result<fs::path, std::string> SharedPlugin::run_project(const RunProjectData &data)
{
    auto [command, root, name, is_debug] = data;
    if (!command)
    {
        using namespace std::string_literals;
        return Err<fs::path>("No command specified"s);
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
        return Err<fs::path>("Cannot find executable file" + result.filename().string() + ".");

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

Result<std::optional<std::string>, std::string> SharedPlugin::get_target(const RunProjectData &data) const
{
    auto [command, root, name, is_debug] = data;
    auto config = data::parse_toml_file<data::Shared>(root / "cup.toml");
    auto unique_suffix = name + "_" + replace(config.project.version, ".", "_");
    if (!command)
        return Ok<std::string>(std::optional<std::string>());
    auto target = *command;
    if (target.starts_with("tests/"))
    {
        auto str = target.substr(6);
        auto filename = split(str, ".")[0];
        return Ok<std::string, std::optional<std::string>>("test_" + filename + '_' + unique_suffix);
    }
    else if (target.starts_with("examples/"))
    {
        auto str = target.substr(9);
        auto filename = split(str, ".")[0];
        return Ok<std::string, std::optional<std::string>>("example_" + filename + '_' + unique_suffix);
    }
    else
    {
        return Err<std::optional<std::string>>("Invalid target: " + target);
    }
    return Ok<std::string, std::optional<std::string>>(std::nullopt);
}

Result<int, std::string> SharedPlugin::show_help(const cmd::Args &command) const
{
    std::cout <<
#include "template/help/built-in/shared.txt"
        ;
    return Ok<std::string>(0);
}
