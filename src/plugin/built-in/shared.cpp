#include "plugin/built-in/shared.h"
#include "log.h"
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include "template.h"

std::string SharedPlugin::getName() const
{
    return "shared";
}

const std::unordered_map<std::string, std::string> SharedPlugin::templates = {
    {
        "cup.toml",
#include "template/cup.toml.txt"
    },
    {
        "export.h",
#include "template/shared/export.h.txt"
    },
    {
        "shared.cpp",
#include "template/shared/shared.cpp.txt"
    },
};

int SharedPlugin::run_new(const NewData &data)
{
#ifdef _DEBUG
    std::cout << "SharedPlugin::run_new: " << data.name << " " << data.type << std::endl;
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
            templates.at("shared.cpp"),
            {
                {"NAME", data.name},
            });
        ofs << file_template.getContent();
    }
    return 0;
}
std::string SharedPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency)
{
    return std::string();
}