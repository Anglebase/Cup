#pragma once

#include "cmd/args.h"
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "plugin/loader.h"
namespace fs = std::filesystem;

class SubCommand
{
public:
    SubCommand(const cmd::Args &args) {}
    virtual int run() = 0;
};

class New : public SubCommand
{
    std::string name;
    std::string type;
    fs::path root;

public:
    New(const cmd::Args &args);
    int run() override;
};

class Help : public SubCommand
{
    static const std::unordered_map<std::string, std::string> help_info;
    std::string key;
    cmd::Args args;

public:
    Help(const cmd::Args &args);
    int run() override;
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

class Run : public Build
{
    std::string args;

public:
    Run(const cmd::Args &args);
    int run() override;
};

class Version : public SubCommand
{
public:
    Version(const cmd::Args &args);
    int run() override;
};

class List : public SubCommand
{
    std::string option;

public:
    List(const cmd::Args &args);
    int run() override;
};

class Install : public SubCommand
{
    std::string url;
    std::optional<std::string> version;

public:
    Install(const cmd::Args &args);
    int run() override;
};

class Uninstall : public SubCommand
{
    std::string url;
    std::string version;

public:
    Uninstall(const cmd::Args &args);
    int run() override;
};

class Clean : public SubCommand
{
    fs::path root;

public:
    Clean(const cmd::Args &args);
    int run() override;
};