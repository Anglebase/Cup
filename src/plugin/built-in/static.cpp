#include "plugin/built-in/static.h"
#include "log.h"
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include "utils/utils.h"
#include "toml/default/static.h"
#include "template.h"
#include "res.h"

std::string StaticPlugin::getName() const
{
    return "static";
}

const std::unordered_map<std::string, std::string> StaticPlugin::templates = {
    {
        "cup.toml",
#include "template/cup.toml.txt"
    },
    {
        "export.h",
#include "template/static/export.h.txt"
    },
    {
        "static.cpp",
#include "template/static/static.cpp.txt"
    },
    {
        "static.cmake",
#include "template/static/static.cmake.txt"
    },
};

int StaticPlugin::run_new(const NewData &data)
{
#ifdef _DEBUG
    std::cout << "StaticPlugin::run_new: " << data.name << " " << data.type << std::endl;
#endif
    const auto project_dir = data.root / data.name;
    LOG_DEBUG(project_dir);
    if (fs::exists(project_dir))
        throw std::runtime_error("The directory '" + data.name + "' already exists.");
    const std::unordered_set<std::string> dirs{"export", "include", "src"};
    for (const auto &dir : dirs)
        fs::create_directories(project_dir / dir);
    const auto cup_toml = project_dir / "cup.toml";
    const auto export_dir = project_dir / "export" / data.name;
    fs::create_directories(export_dir);
    const auto source_dir = project_dir / "src";
    {
        std::ofstream ofs(cup_toml);
        auto file_template = FileTemplate(
            templates.at("cup.toml"),
            {
                {"NAME", data.name},
                {"TYPE", data.type},
            });
        ofs << file_template.getContent();
    }
    {
        std::ofstream ofs(export_dir / (data.name + ".h"));
        auto upper_name = data.name;
        std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);
        auto file_template = FileTemplate(
            templates.at("export.h"),
            {
                {"NAME", data.name},
                {"UNAME", upper_name},
            });
        ofs << file_template.getContent();
    }
    {
        std::ofstream ofs(source_dir / (data.name + ".cpp"));
        auto file_template = FileTemplate(
            templates.at("static.cpp"),
            {
                {"NAME", data.name},
            });
        ofs << file_template.getContent();
    }
    return 0;
}

std::string StaticPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency)
{
    // Parse cup.toml
    auto toml_config = data::Deserializer<data::Static>::deserialize(
        toml::parse(read_file(ctx.current_dir / "cup.toml")));
    // Find all source files
    auto source_files = find_all_src(ctx.current_dir / "src");
    // Find all include directories
    std::vector<fs::path> include_dirs{ctx.current_dir / "include"};
    if (toml_config.build.has_value() && toml_config.build->includes.has_value())
        for (const auto &dir : *toml_config.build->includes)
            include_dirs.push_back(dir);
    // Find all defines
    std::vector<std::string> defines;
    if (toml_config.build.has_value() && toml_config.build->defines.has_value())
        for (const auto &define : *toml_config.build->defines)
            defines.push_back(define);
    if (is_dependency)
        for (const auto &feature : ctx.features)
            defines.push_back(feature);
    // Find all Link directories
    std::vector<fs::path> link_dirs;
    if (toml_config.build.has_value() && toml_config.build->link_dirs.has_value())
        for (const auto &dir : *toml_config.build->link_dirs)
            link_dirs.push_back(dir);
    // Find all dependencies
    std::vector<std::string> libs;
    if (toml_config.dependencies.has_value())
        for (const auto &[name, _] : *toml_config.dependencies)
            libs.push_back(name);
    if (toml_config.build.has_value() && toml_config.build->link_libs.has_value())
        for (const auto &lib : *toml_config.build->link_libs)
            libs.push_back(lib);
    // Find all example main files
    auto example_mains = find_all_example_main(ctx.current_dir / "example");
    // Replace placeholders in CMakeLists.txt
    std::unordered_map<std::string, std::string> replacements = {
        {
            "OUT_NAME",
            ctx.name,
        },
        {
            "SOURCES",
            join(source_files.begin(), source_files.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.string()) + '"'; }),
        },
        {
            "DEFINES",
            join(defines.begin(), defines.end(), " ", [](const std::string &s)
                 { return "-D" + s; }),
        },
        {
            "INCLUDE_DIRS",
            join(include_dirs.begin(), include_dirs.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.string()) + '"'; }),
        },
        {
            "EXPORT_INC",
            '"' + replace((ctx.current_dir / "export").string()) + '"',
        },
        {
            "LINK_DIRS",
            join(link_dirs.begin(), link_dirs.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.string()) + '"'; }),
        },
        {
            "LINK_LIBS",
            join(libs, " "),
        },
        {
            "AS_DEP",
            is_dependency ? "ON" : "OFF",
        },
        {
            "EXAMPLE_MAINS",
            join(example_mains.begin(), example_mains.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.string()) + '"'; }),
        },
        {
            "EXAMPLE_OUTDIR",
            '"' + replace((Resource::target(ctx.current_dir) / "bin" / "example").string()) + '"',
        },
        {
            "LIB_OUTDIR",
            '"' + replace((Resource::target(ctx.current_dir) / "lib").string()) + '"',
        },
    };
    // Gernerate the cmake file content.
    auto file_template = FileTemplate(
        templates.at("static.cmake"),
        replacements);
    ctx.set_cmake_version(3, 10);
    return file_template.getContent();
}

fs::path StaticPlugin::run_project(const RunProjectData &data)
{
    return fs::path();
}
std::optional<std::string> StaticPlugin::get_target(const RunProjectData &data) const
{
    return std::optional<std::string>();
}