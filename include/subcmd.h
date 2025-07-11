#pragma once

#include "build.h"

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
    bool all{false};

public:
    Clean(const cmd::Args &args);
    int run() override;
};