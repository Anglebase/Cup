#include "plugin/built-in/module.h"
#include "log.h"
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include "template.h"

std::string ModulePlugin::getName() const
{
    return "module";
}

const std::unordered_map<std::string, std::string> ModulePlugin::templates = {
    {
        "cup.toml",
#include "template/cup.toml.txt"
    },
    {
        "export.h",
#include "template/module/export.h.txt"
    },
    {
        "module.cpp",
#include "template/module/module.cpp.txt"
    },
};

int ModulePlugin::run_new(const NewData &data)
{
#ifdef _DEBUG
    std::cout << "ModulePlugin::run_new: " << data.name << " " << data.type << std::endl;
#endif
    const auto project_dir = data.root / data.name;    const std::unordered_set<std::string> dirs{"export", "include", "src"};
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
            templates.at("module.cpp"),
            {
                {"NAME", data.name},
            });
        ofs << file_template.getContent();
    }
    return 0;
}
std::string ModulePlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency)
{
    return std::string();
}