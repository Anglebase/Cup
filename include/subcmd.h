#pragma once

#include "cmd/args.h"
#include <unordered_map>
#include <filesystem>
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

public:
    Help(const cmd::Args &args);
    int run() override;
};

class Build : public SubCommand
{
    fs::path root;
    bool is_release{false};

public:
    Build(const cmd::Args &args);
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
public:
    List(const cmd::Args &args);
    int run() override;
};

class Install : public SubCommand
{
public:
    Install(const cmd::Args &args);
    int run() override;
};

class Uninstall : public SubCommand
{
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