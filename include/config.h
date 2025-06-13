#pragma once

#include "toml++/toml.hpp"
#include "utils.h"
#include "log.h"
#include "configinfo.h"
#include <filesystem>
#include <variant>
#include <unordered_map>
namespace fs = std::filesystem;

struct ConfigInfo;

class Config
{
    toml::table table_;
    friend struct ConfigInfo;

public:
    fs::path path;

public:
    Config(const fs::path &project);
    ~Config();
    ConfigInfo *config;
};

#ifdef DEBUG_CONFIG
#include <iostream>
std::ostream &operator<<(std::ostream &os, const ConfigInfo &config)
{
    os << "name: " << config.name << std::endl;
    os << "version: " << config.version << std::endl;
    os << "description: " << config.description << std::endl;
    os << "authors: " << std::endl;
    for (auto &author : config.authors)
        os << "  - " << author << std::endl;
    os << "license: " << config.license << std::endl;
    os << "build.target: " << config.build.target << std::endl;
    os << "build.stdc: " << config.build.stdc << std::endl;
    os << "build.stdcxx: " << config.build.stdcxx << std::endl;
    os << "build.define: " << std::endl;
    for (auto &define : config.build.define)
        os << "  - " << define << std::endl;
    os << "build.include: " << std::endl;
    for (auto &include : config.build.include)
        os << "  -I " << include << std::endl;
    os << "link: " << std::endl;
    for (auto &lib : config.link.libs)
        os << "  -l " << lib << std::endl;
    for (auto &path : config.link.paths)
        os << "  -L " << path << std::endl;
    os << "dependencies: " << std::endl;
    for (auto &dependency : config.dependencies)
        os << "  - " << dependency.first << " -> " << dependency.second.path.value_or(fs::path()) << std::endl;
    return os;
}
#endif