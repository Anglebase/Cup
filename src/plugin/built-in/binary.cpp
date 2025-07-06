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
        else if (entry.is_regular_file())
            files.push_back(entry.path());
    }
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

std::string BinaryPlugin::gen_cmake(const CMakeContext &ctx, bool is_dependency, std::optional<std::string> &)
{
    if (is_dependency)
        throw std::runtime_error("'binary' project cannot be a dependency.");
    auto [name, _1, current_dir, root_dir, _2] = ctx;
    auto project = current_dir;
    auto src = project / "src";
    auto include = project / "include";
    auto config = data::Deserializer<data::Binary>::deserialize(
        toml::parse_file((project / "cup.toml").string()));
    return FileTemplate{
#include "template/binary/binary.cmake"
        ,
        {
            {
                "MAIN_FILE",
                [&]
                {
                    if (!fs::exists(src))
                        throw std::runtime_error("Cannot find 'src' directory.");
                    for (const auto &entry : fs::directory_iterator(src))
                        if (entry.is_regular_file() && entry.path().stem() == "main")
                            return '"' + replace(entry.path().string()) + '"';
                    throw std::runtime_error("Cannot find main file in 'src' directory.");
                }(),
            },
            {
                "SOURCES",
                [&]
                {
                    if (!fs::exists(src))
                        throw std::runtime_error("Cannot find 'src' directory.");
                    std::vector<fs::path> sources;
                    this->_get_source_files(src, sources);
                    auto last = std::remove_if(sources.begin(), sources.end(), [&](const fs::path &p)
                                               { return p.stem() == "main" && fs::equivalent(p.parent_path(), src); });
                    sources.erase(last, sources.end());
                    return join(sources.begin(), sources.end(), " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "UNIQUE_SUFFIX",
                replace(config.project.version, ".", "_"),
            },
            {
                "TARGET_NAME",
                name,
            },
            {
                "INCLUDE_DIRS",
                '"' + replace(include.string()) + '"' +
                    (config.build &&
                             config.build->includes
                         ? join(config.build->includes->begin(), config.build->includes->end(), " ",
                                [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; })
                         : ""),
            },
            {
                "LIBRARY_DIRS",
                config.build && config.build->link_dirs
                    ? join(config.build->link_dirs->begin(), config.build->link_dirs->end(), " ",
                           [](const fs::path &s)
                           { return '"' + replace(s.string()) + '"'; })
                    : "",
            },
            {
                "LIBRARIES",
                config.build && config.build->link_libs
                    ? join(*config.build->link_libs, " ")
                    : "",
            },
            {
                "DEFINES",
                config.build && config.build->defines
                    ? join(config.build->defines->begin(), config.build->defines->end(), " ",
                           [](const std::string &s)
                           { return "-D" + s; })
                    : "",
            },
            {
                "COPTIONS",
                config.build && config.build->compiler_options
                    ? join(config.build->compiler_options->begin(), config.build->compiler_options->end(), " ",
                           [](const std::string &s)
                           { return '"' + s + '"'; })
                    : "",
            },
            {
                "LOPTIONS",
                config.build && config.build->link_options
                    ? join(config.build->link_options->begin(), config.build->link_options->end(), " ",
                           [](const std::string &s)
                           { return '"' + s + '"'; })
                    : "",
            },
            {
                "MAIN_OUTDIR",
                '"' + replace(Resource::bin(project).string()) + '"',
            },
            {
                "STDC",
                config.build && config.build->stdc ? std::to_string(*config.build->stdc) : "",
            },
            {
                "STDCPP",
                config.build && config.build->stdcxx ? std::to_string(*config.build->stdcxx) : "",
            },
            {
                "BIN_MAIN_FILES",
                [&]
                {
                    if (!fs::exists(src / "bin"))
                        return std::string();
                    std::vector<fs::path> files;
                    for (const auto &entry : fs::directory_iterator(src / "bin"))
                        if (entry.is_regular_file())
                            files.push_back(entry.path());
                    return join(files.begin(), files.end(), " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "TEST_MAIN_FILES",
                [&]
                {
                    if (!fs::exists(project / "tests"))
                        return std::string();
                    std::vector<fs::path> files;
                    for (const auto &entry : fs::directory_iterator(project / "tests"))
                        if (entry.is_regular_file())
                            files.push_back(entry.path());
                    return join(files.begin(), files.end(), " ", [](const fs::path &p)
                                { return '"' + replace(p.string()) + '"'; });
                }(),
            },
            {
                "BIN_OUTDIR",
                '"' + replace((Resource::bin(project) / "bin").string()) + '"',
            },
            {
                "TEST_OUTDIR",
                '"' + replace((Resource::bin(project) / "test").string()) + '"',
            },
        },
    }
        .getContent();
}

fs::path BinaryPlugin::run_project(const RunProjectData &data, std::optional<std::string> &)
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
        throw std::runtime_error("Cannot find executable file" + result.filename().string() + ".");
    return result;
}

std::optional<std::string> BinaryPlugin::get_target(const RunProjectData &data, std::optional<std::string> &) const
{
    auto config = data::Deserializer<data::Binary>::deserialize(
        toml::parse_file((data.root / "cup.toml").string()));
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
    else if (target.starts_with("test/"))
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
        throw std::runtime_error("Invalid target: " + target);
    }
}

int BinaryPlugin::show_help(const cmd::Args &command, std::optional<std::string> &) const
{
    std::cout <<
#include "template/help/built-in/binary.txt"
        ;
    return 0;
}
