#pragma once

#include "cmd/args.h"
#include "build.h"
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "plugin/loader.h"
#include "toml/dependency.h"
namespace fs = std::filesystem;

class SubCommand
{
public:
    SubCommand(const cmd::Args &args) {}
    virtual int run() = 0;
};

class Build : public SubCommand
{
protected:
    bool is_release{false};
    fs::path root;
    std::optional<std::string> target;
    std::optional<std::string> command;

public:
    Build(const cmd::Args &args);
    int run() override;
};

std::pair<fs::path, std::string> get_path(const data::Dependency &dep, bool download = true,
    const std::optional<fs::path> &root = std::nullopt);