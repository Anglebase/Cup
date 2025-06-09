#pragma once

#include "cmd.h"
#include "build.h"
#include <filesystem>
#include <optional>
namespace fs = std::filesystem;

class NewCmd
{
    std::string name;
    std::optional<fs::path> at;
    std::optional<std::string> target;

public:
    NewCmd(const SysArgs &args);
    int run();
};

class CleanCmd
{
    std::optional<fs::path> at;

public:
    CleanCmd(const SysArgs &args);
    int run();
};

class BuildCmd
{
protected:
    std::optional<fs::path> at;
    BuildType config;
    SysArgs args;

public:
    BuildCmd(const SysArgs &args);
    int run();
};

class RunCmd : public BuildCmd
{
    std::vector<std::string> run_args;
    fs::path run_target;

public:
    RunCmd(const SysArgs &args);
    int exec();
};

class ListCmd
{
    std::optional<fs::path> at;
    std::string option;

public:
    ListCmd(const SysArgs &args);
    int run();
};