#pragma once

#include "cmd.h"
#include "config.h"
#include "cmake.h"
#include <fstream>
#include <filesystem>
#include <functional>
namespace fs = std::filesystem;

enum class BuildType
{
    Debug,
    Release,
};

struct BuildInfo
{
    BuildType type;
    fs::path project_dir;
    fs::path build_dir;
    fs::path target_dir;
    std::optional<fs::path> build_target;

    BuildInfo(const fs::path &project_dir, const SysArgs &args);
};

struct Task {
    Version version;
    std::function<void(cmake::Generator&)> func;
};

class Build
{
    BuildInfo info;
    ConfigInfo config;
    std::unordered_set<std::string> build_depends;
    std::optional<std::string> cmake_gen;
    std::vector<std::string> stack;

    std::vector<std::pair<std::string, Task>> tasks;

public:
    Build(const BuildInfo &info, const ConfigInfo &config);

private:
    void generate_cmake_root(cmake::Generator &gen);
    void generate_cmake_sub(const Dependency &cup, cmake::Generator &gen);

public:
    void generate_build(std::ofstream &ofs);
    int build();
};