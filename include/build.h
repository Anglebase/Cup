#pragma once

#include "cmd/args.h"
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "plugin/loader.h"
#include "toml/dependency.h"
#include <iostream>
#include <fstream>
namespace fs = std::filesystem;

class SubCommand
{
public:
    SubCommand(const cmd::Args &args) {}
    virtual int run() = 0;
};

struct VersionInfo
{
    int x;
    int y;
    int z;

    bool operator>(const VersionInfo &other) const;
    friend std::ostream &operator<<(std::ostream &os, const VersionInfo &v);
    static VersionInfo parse(const std::string &str);
};

struct CMakeOutBlock
{
    std::string unique;
    std::string content;
    std::string content_global;
    VersionInfo version;
    fs::path path;
};

class CMakeOutContent
{
    std::vector<CMakeOutBlock> content;

public:
    std::string push(const CMakeOutBlock &block);
    void write_to(std::ofstream &os);
    void write_global_to(std::ofstream &os);
};

struct FromParent
{
    std::string key;
    std::vector<std::string> features;
    fs::path root_dir;
};

class Build : public SubCommand
{
    std::string generator;
    int64_t jobs;
    std::pair<int, int> cmake_version{3, 10};
    std::string name;
    CMakeOutContent output;
    std::vector<std::string> cycle_check;

    std::string generate_cmake(const fs::path &cup, const std::optional<FromParent> &info = std::nullopt);
protected:
    bool is_release{false};
    fs::path root;
    std::optional<std::string> target;
    std::optional<std::string> command;
    std::optional<fs::path> compile_commands;

public:
    Build(const cmd::Args &args);
    int run() override;
};

std::pair<fs::path, std::string> get_path(const data::Dependency &dep, bool download = true,
                                          const std::optional<fs::path> &root = std::nullopt);