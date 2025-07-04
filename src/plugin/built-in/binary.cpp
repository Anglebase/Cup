#include "plugin/built-in/binary.h"
#include <unordered_set>
#include <fstream>
#include "log.h"
#include "toml/default/binary.h"
#include "utils/utils.h"
#include "res.h"

std::string BinaryPlugin::getName() const
{
    return "binary";
}

const std::unordered_map<std::string, std::string> BinaryPlugin::templates = {
    {
        "main.cpp",
#include "template/binary/main.cpp.txt"
    },
    {
        "binary.cmake",
#include "template/binary/binary.cmake.txt"
    },
    {
        "cup.toml",
#include "template/cup.toml.txt"
    },
};

int BinaryPlugin::run_new(const NewData &data)
{
#ifdef _DEBUG
    std::cout << "BinaryPlugin::run_new: " << data.name << " " << data.type << std::endl;
#endif
    const auto project_dir = data.root / data.name;
    LOG_DEBUG(project_dir);
    if (fs::exists(project_dir))
        throw std::runtime_error("The directory '" + data.name + "' already exists.");
    const std::unordered_set<std::string> dirs{"include", "src"};
    for (const auto &dir : dirs)
        fs::create_directories(project_dir / dir);
    const auto source_dir = project_dir / "src";
    const auto include_dir = project_dir / "include";
    const auto main_cpp = source_dir / "main.cpp";
    const auto cup_toml = project_dir / "cup.toml";
    {
        std::ofstream ofs(main_cpp);
        ofs << templates.at("main.cpp");
    }
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
    return 0;
}

void _get_all_src_files(const fs::path &dir, std::vector<fs::path> &files)
{
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_directory() && entry.path().filename() != "bin")
        {
            _get_all_src_files(entry.path(), files);
        }
        else if (entry.is_regular_file())
        {
            if (entry.path().stem() != "main")
                files.push_back(entry.path());
        }
    }
}

std::vector<fs::path> get_src_files(const fs::path &src)
{
    std::vector<fs::path> files;
    _get_all_src_files(src, files);
    return files;
}

void _get_bin_main_files(const fs::path &dir, std::vector<fs::path> &files)
{
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file())
        {
            files.push_back(entry.path());
        }
    }
}

std::vector<fs::path> get_main_files(const fs::path &src)
{
    std::vector<fs::path> files;
    for (const auto &entry : fs::directory_iterator(src))
    {
        if (entry.is_directory() && entry.path().filename() == "bin")
        {
            _get_bin_main_files(entry.path(), files);
        }
        else if (entry.path().stem() == "main")
        {
            files.push_back(entry.path());
        }
    }
    return files;
}

std::string BinaryPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency)
{
    if (is_dependency)
        throw std::runtime_error("Project 'binary' cannot be used as a dependency.");
    // Parse the toml config file
    auto toml_config = data::Deserializer<data::Binary>::deserialize(
        toml::parse_file((ctx.root_dir / "cup.toml").string()));
    // Find all source files
    auto src_files = get_src_files(ctx.root_dir / "src");
    // Find all main files
    auto main_files = get_main_files(ctx.root_dir / "src");
    // Find all include directories
    auto include_dirs = std::vector<fs::path>{ctx.root_dir / "include"};
    if (toml_config.build.has_value() && toml_config.build->includes.has_value())
    {
        for (const auto &inc : *toml_config.build->includes)
        {
            if (inc.is_absolute())
                include_dirs.push_back(inc);
            else
                include_dirs.push_back(ctx.root_dir / inc);
        }
    }
    // Find all link directories
    auto link_dirs = std::vector<fs::path>{};
    if (toml_config.build.has_value() && toml_config.build->link_dirs.has_value())
    {
        for (const auto &ld : *toml_config.build->link_dirs)
        {
            if (ld.is_absolute())
                link_dirs.push_back(ld);
            else
                link_dirs.push_back(ctx.root_dir / ld);
        }
    }
    // Find all link libraries
    auto link_libs = std::vector<std::string>{};
    if (toml_config.dependencies.has_value())
        for (const auto &[name, _] : toml_config.dependencies.value())
            link_libs.push_back(name);
    if (toml_config.build.has_value() && toml_config.build->link_libs.has_value())
        for (const auto &ll : *toml_config.build->link_libs)
            link_libs.push_back(ll);
    // Find all defines
    auto defines = std::vector<std::string>{};
    if (toml_config.build.has_value() && toml_config.build->defines.has_value())
        for (const auto &d : *toml_config.build->defines)
            defines.push_back(d);
    // Find all compiler options
    auto compiler_options = std::vector<std::string>{};
    if (toml_config.build.has_value() && toml_config.build->compiler_options.has_value())
        for (const auto &co : *toml_config.build->compiler_options)
            compiler_options.push_back(co);
    // Find all link options
    auto link_options = std::vector<std::string>{};
    if (toml_config.build.has_value() && toml_config.build->link_options.has_value())
        for (const auto &lo : *toml_config.build->link_options)
            link_options.push_back(lo);
    // Generate the replacements.
    std::unordered_map<std::string, std::string> replacements = {
        {
            "OUT_NAME",
            toml_config.project.name,
        },
        {
            "SRC_FILES",
            join(src_files.begin(), src_files.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.lexically_normal().string()) + '"'; }),
        },
        {
            "INC_DIRS",
            join(include_dirs.begin(), include_dirs.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.lexically_normal().string()) + '"'; }),
        },
        {
            "LINK_DIRS",
            join(link_dirs.begin(), link_dirs.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.lexically_normal().string()) + '"'; }),
        },
        {
            "MAIN_FILES",
            join(main_files.begin(), main_files.end(), " ", [](const fs::path &p)
                 { return '"' + replace(p.lexically_normal().string()) + '"'; }),
        },
        {
            "LINK_LIBS",
            join(link_libs, " "),
        },
        {
            "DEFINES",
            join(defines.begin(), defines.end(), " ", [](const std::string &s)
                 { return "-D" + s; }),
        },
        {
            "COMPILER_OPTIONS",
            join(compiler_options, " "),
        },
        {
            "LINK_OPTIONS",
            join(link_options, " "),
        },
        {
            "BIN_OUT_DIR",
            '"' + replace((Resource::target(ctx.root_dir) / "bin").lexically_normal().string()) + '"',
        }};
    // Gernerate the cmake file content.
    auto file_template = FileTemplate(
        templates.at("binary.cmake"),
        replacements);
    ctx.set_cmake_version(3, 10);
    return file_template.getContent();
}

fs::path BinaryPlugin::run_project(const RunProjectData &data)
{
    auto path = Resource::bin(data.root) / data.command.value_or(data.name);
#ifdef _WIN32
    path.replace_extension(".exe");
#else
    path.replace_extension("");
#endif
    if (!fs::exists(path))
        path = path.parent_path() / (data.is_debug ? "Debug" : "Release") / path.filename();
    return path;
}
std::optional<std::string> BinaryPlugin::get_target(const RunProjectData &data) const
{
    return data.command
               ? fs::path(data.command.value()).stem().string()
               : std::optional<std::string>{};
}