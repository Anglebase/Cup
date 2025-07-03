#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
namespace fs = std::filesystem;
#ifdef _DEBUG
#include <iostream>
#endif

struct NewData
{
    std::string name;
    std::string type;
    fs::path root;
};

/// @brief Context for generating CMakeLists.txt
/// @note All of the paths are absolute paths.
struct CMakeContext
{
    std::string project_name;
    mutable std::pair<int, int> cmake_version;
    fs::path current_dir;
    fs::path root_dir;
    std::vector<std::string> features;

    void set_cmake_version(int major, int minor) const
    {
        if (major > cmake_version.first || (major == cmake_version.first && minor > cmake_version.second))
            cmake_version = {major, minor};
    }
};

struct Info
{
    bool as_dependency = true;
};

class IPlugin
{
public:
    virtual std::string getName() const = 0;
    virtual int run_new(const NewData &data) = 0;
    virtual std::string gen_cmake(const CMakeContext &context, bool is_dependency) = 0;
};

#define EXPORT_PLUGIN(impl)                                    \
    extern "C"                                                 \
    {                                                          \
        IPlugin *createPlugin() { return new impl(); }         \
        void destroyPlugin(IPlugin *plugin) { delete plugin; } \
    }



inline void _find_all_src(const fs::path &dir, std::vector<fs::path> &src_files)
{
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_directory())
            _find_all_src(entry.path(), src_files);
        else if (entry.is_regular_file())
            src_files.push_back(entry.path());
    }
}

inline std::vector<fs::path> find_all_src(const fs::path &src)
{
    std::vector<fs::path> src_files;
    _find_all_src(src, src_files);
    return src_files;
}

inline std::vector<fs::path> find_all_example_main(const fs::path &example)
{
    std::vector<fs::path> main_files;
    if (!fs::exists(example))
        return main_files;
    for (const auto &entry : fs::directory_iterator(example))
        if (entry.is_regular_file())
            main_files.push_back(entry.path());
    return main_files;
}