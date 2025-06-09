#pragma once

#include "cmd.h"
#include "config.h"
#include "cmake.h"
#include <fstream>
#include <filesystem>
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

class Build
{
    BuildInfo info;
    ConfigInfo config;
    std::unordered_set<std::string> build_depends;

public:
    Build(const BuildInfo &info, const ConfigInfo &config);

private:
    void generate_cmake_root(cmake::Generator &gen);
    void generate_cmake_sub(const fs::path &path, cmake::Generator &gen);

public:
    void generate_build(std::ofstream &ofs);
    int build();
};