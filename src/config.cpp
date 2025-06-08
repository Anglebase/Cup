#include "config.h"
#include <fstream>

Config::Config(const fs::path &project)
{
    auto cup_toml = project / "cup.toml";
    if (!fs::exists(cup_toml))
        throw std::runtime_error("'" + project.string() + "' is not a valid cup project.");
    std::ifstream ifs(cup_toml.string());
    this->table_ = toml::parse(ifs);
    this->path = cup_toml;
    this->config = new ConfigInfo(*this);
}

Config::~Config()
{
    delete this->config;
}

ConfigInfo::ConfigInfo(const Config &config)
{
    this->name = config.get<std::string>("name");
    this->version = config.get<std::string>("version");
    this->description = config.need<std::string>("description", "", false);
    if (config.table_.contains("authors") && config.table_.at("authors").is_array())
    {
        auto authors = *config.table_.at("authors").as_array();
        for (auto &&author : authors)
        {
            if (author.is_string())
            {
                auto author_str = author.value<std::string>().value();
                this->authors.push_back(author_str);
            }
            else
            {
                throw std::runtime_error(config.path.string() + ": 'author' must be an array of strings.");
            }
        }
    }
    this->license = config.need<std::string>("license", "", false);
    this->build.target = config.get<std::string>("build.target");
    this->build.stdc = config.need<int>("build.stdc", 17, false);
    this->build.stdcxx = config.need<int>("build.stdcxx", 20, false);
    this->build.generator = config.need<std::string>("build.generator", "", false);
    this->build.jobs = config.need<unsigned int>("build.jobs", 1, false);
    if (config.table_.contains("build") && config.table_.at("build").is_table())
    {
        auto build_table = *config.table_.at("build").as_table();
        if (build_table.contains("define") && build_table.at("define").is_array())
        {
            auto defines = *build_table.at("define").as_array();
            for (auto &&define : defines)
            {
                if (define.is_string())
                {
                    auto def = define.value<std::string>().value();
                    this->build.define.push_back(def);
                }
                else
                {
                    throw std::runtime_error(config.path.string() + ": build.define must be an array of strings.");
                }
            }
        }
        if (build_table.contains("include") && build_table.at("include").is_array())
        {
            auto includes = *build_table.at("include").as_array();
            for (auto &&include : includes)
            {
                if (include.is_string())
                {
                    fs::path inc = include.value<std::string>().value();
                    this->build.include.push_back(inc.is_relative() ? config.path.parent_path() / inc : inc);
                }
                else
                {
                    throw std::runtime_error(config.path.string() + ": build.include must be an array of strings.");
                }
            }
        }
    }
    if (config.table_.contains("dependencies") && config.table_.at("dependencies").is_table())
    {
        auto dependencies = *config.table_.at("dependencies").as_table();
        for (auto &&[name, table] : dependencies)
        {
            auto key = std::string(name.str());
            auto path = config.need<std::string>("dependencies." + key + ".path", "", false);
            if (path.empty())
                throw std::runtime_error(config.path.string() + ": 'dependencies." + key + "' does not have the 'path'.");
            this->dependencies.insert({key, CupProject{.path = path}});
        }
    }
    if (config.table_.contains("link") && config.table_.at("link").is_table())
    {
        auto link_table = *config.table_.at("link").as_table();
        for (auto &&[name, dir] : link_table)
        {
            auto key = std::string(name.str());
            if (dir.is_string())
            {
                fs::path dir_str = dir.value<std::string>().value();
                if (!dir_str.empty())
                    this->link.insert({key, dir_str.is_relative() ? config.path.parent_path() / dir_str : dir_str});
            }
            else
            {
                throw std::runtime_error(config.path.string() + ": 'link." + key + "' must be a string.");
            }
        }
    }
}
