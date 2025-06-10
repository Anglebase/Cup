#pragma once

#include "toml++/toml.hpp"
#include "utils.h"
#include "log.h"
#include <filesystem>
#include <variant>
#include <unordered_map>
#include <unordered_set>
namespace fs = std::filesystem;

struct ConfigInfo;

class Config
{
    toml::table table_;
    fs::path path;
#ifdef DEBUG_CONFIG
public:
#endif
    // 断言配置项必定存在并获取，若不存在则抛出异常
    template <class T>
    T get(const std::string &key_dir, std::string_view error_message_view = "") const
    {
        std::string error_message;
        if (error_message_view.empty())
        {
            std::ostringstream oss;
            oss << "Error: Unable to find configuration item: '" << key_dir
                << "' in file: '" << this->path << "'.";
            error_message = oss.str();
        }
        else
            error_message = std::string(error_message_view);
        auto keys = split(key_dir, ".");
        auto table_keys = std::vector<std::string>(keys.begin(), keys.end() - 1);
        auto table = &this->table_;
        for (auto key : table_keys)
        {
            if (!table->contains(key) || !table->at(key).is_table())
                throw std::runtime_error(error_message);
            table = table->at(key).as_table();
        }
        if (!table->contains(keys.back()))
            throw std::runtime_error(error_message);
        toml::v3::optional<T> value = table->at(keys.back()).value<T>();
        if (!value.has_value())
            throw std::runtime_error(error_message);
        return value.value();
    }

    // 查询配置项是否存在，若不存在则返回默认值
    template <class T>
    T need(const std::string &key_dir, T default_, bool show_warning = true, std::string_view warning_message_view = "") const
    {
        std::string warning_message;
        if (warning_message_view.empty())
        {
            std::ostringstream oss;
            oss << "Warning: Unable to find configuration item: '" << key_dir
                << "' in file: '" << this->path
                << "'. It will use the default value: '" << default_ << "'.";
            warning_message = oss.str();
        }
        else
            warning_message = std::string(warning_message_view);
        auto keys = split(key_dir, ".");
        auto table_keys = std::vector<std::string>(keys.begin(), keys.end() - 1);
        auto table = &this->table_;
        for (auto key : table_keys)
        {
            if (!table->contains(key) || !table->at(key).is_table())
            {
                if (show_warning)
                    LOG_WARN(warning_message);
                return default_;
            }
            table = table->at(key).as_table();
        }
        if (!table->contains(keys.back()))
        {
            if (show_warning)
                LOG_WARN(warning_message);
            return default_;
        }
        toml::v3::optional<T> value = table->at(keys.back()).value<T>();
        if (!value.has_value())
        {
            if (show_warning)
                LOG_WARN(warning_message);
            return default_;
        }
        return value.value();
    }

public:
    Config(const fs::path &project);
    ~Config();
    ConfigInfo *config;
    friend struct ConfigInfo;
};

struct CupProject
{
    fs::path path;
    std::vector<std::string> features;
};

struct Qt
{
    std::string version;
    std::unordered_set<std::string> modules;
    std::unordered_set<std::string> flags;
};

struct ConfigInfo
{
    std::string name;
    std::string version;
    std::string description;
    std::vector<std::string> authors;
    std::string license;
    struct Build
    {
        std::string target;
        std::string generator;
        int stdc;
        int stdcxx;
        std::vector<std::string> define;
        std::vector<fs::path> include;
        unsigned int jobs;
        struct Options
        {
            std::vector<std::string> compile;
            std::vector<std::string> link;
        } options;
        struct ToolChain
        {
            std::optional<std::string> cc;
            std::optional<std::string> cxx;
            std::optional<std::string> asm_;
            std::optional<std::string> ld;
        } toolchain;
        struct System
        {
            std::optional<std::string> name;
            std::optional<std::string> processor;
        } system;
        std::optional<std::string> suffix;
    } build;
    std::unordered_map<std::string, fs::path> link;
    std::unordered_map<std::string, CupProject> dependencies;

    std::optional<Qt> qt;

    ConfigInfo(const Config &table);
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
    for (auto &link : config.link)
        os << "  -l " << link.first << " -> " << link.second << std::endl;
    os << "dependencies: " << std::endl;
    for (auto &dependency : config.dependencies)
        os << "  - " << dependency.first << " -> " << dependency.second.path << std::endl;
    return os;
}
#endif