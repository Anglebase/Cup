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
