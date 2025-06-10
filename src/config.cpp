#include "config.h"
#include <fstream>
#include <unordered_set>

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

fs::path parser_env(const std::string &str)
{
    if (str.empty())
        return fs::path();
    if (!str.starts_with("$env:"))
        return fs::path(str).lexically_normal();
    auto raw_path = fs::path(str.substr(5));
    auto first = *raw_path.begin();
    auto env_name = first.string();
    if (env_name.find("/") != std::string::npos || env_name.find("\\") != std::string::npos || env_name.find("..") != std::string::npos)
        throw std::runtime_error("Invalid environment variable name.");
    auto env = std::getenv(env_name.c_str());
    if (!env)
        throw std::runtime_error("Environment variable not found.");
    return fs::path(std::string(env) + raw_path.string().substr(first.string().size())).lexically_normal();
}

void load_options(const toml::table &build_table, ConfigInfo::Build::Options &options, const Config &config)
{
    if (build_table.contains("options") && build_table.at("options").is_table())
    {
        auto options_table = *build_table.at("options").as_table();
        if (options_table.contains("compile") && options_table.at("compile").is_array())
        {
            auto compile_options = *options_table.at("compile").as_array();
            for (auto &&option : compile_options)
            {
                if (option.is_string())
                {
                    auto opt = option.value<std::string>().value();
                    if(opt.starts_with("$env:"))
                        opt = replace_all(parser_env(opt).string(), "\\", "/");
                    options.compile.push_back(opt);
                }
                else
                {
                    throw std::runtime_error(config.path.string() + ": build.options.compile must be an array of strings.");
                }
            }
        }
        if (options_table.contains("link") && options_table.at("link").is_array())
        {
            auto link_options = *options_table.at("link").as_array();
            for (auto &&option : link_options)
            {
                if (option.is_string())
                {
                    auto opt = option.value<std::string>().value();
                    if(opt.starts_with("$env:"))
                        opt = replace_all(parser_env(opt).string(), "\\", "/");
                    options.link.push_back(opt);
                }
                else
                {
                    throw std::runtime_error(config.path.string() + ": build.options.link must be an array of strings.");
                }
            }
        }
    }
}

void load_define(const toml::table &build_table, std::vector<std::string> &define_, const Config &config)
{
    if (build_table.contains("define") && build_table.at("define").is_array())
    {
        auto defines = *build_table.at("define").as_array();
        for (auto &&define : defines)
        {
            if (define.is_string())
            {
                auto def = define.value<std::string>().value();
                define_.push_back(def);
            }
            else
            {
                throw std::runtime_error(config.path.string() + ": build.define must be an array of strings.");
            }
        }
    }
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

    auto cc = config.need<std::string>("build.toolchain.cc", "", false);
    auto cxx = config.need<std::string>("build.toolchain.cxx", "", false);
    auto asm_ = config.need<std::string>("build.toolchain.asm", "", false);
    auto ld = config.need<std::string>("build.toolchain.ld", "", false);
    if (!cc.empty())
        this->build.toolchain.cc = cc;
    if (!cxx.empty())
        this->build.toolchain.cxx = cxx;
    if (!asm_.empty())
        this->build.toolchain.asm_ = asm_;
    if (!ld.empty())
        this->build.toolchain.ld = ld;
    auto suffix = config.need<std::string>("build.suffix", "", false);
    if (!suffix.empty())
        this->build.suffix = suffix;

    if (config.table_.contains("build") && config.table_.at("build").is_table())
    {
        auto build_table = *config.table_.at("build").as_table();
        load_define(build_table, this->build.define, config);
        if (build_table.contains("include") && build_table.at("include").is_array())
        {
            auto includes = *build_table.at("include").as_array();
            for (auto &&include : includes)
            {
                if (include.is_string())
                {
                    fs::path inc = parser_env(include.value<std::string>().value());
                    this->build.include.push_back(inc.is_relative() ? config.path.parent_path() / inc : inc);
                }
                else
                {
                    throw std::runtime_error(config.path.string() + ": build.include must be an array of strings.");
                }
            }
        }
        load_options(build_table, this->build.options, config);
        if (build_table.contains("system") && build_table.at("system").is_table())
        {
            auto name = config.need<std::string>("build.system.name", "", false);
            if (!name.empty())
                this->build.system.name = name;
            auto processor = config.need<std::string>("build.system.processor", "", false);
            if (!processor.empty())
                this->build.system.processor = processor;
        }
        if (build_table.contains("debug") && build_table.at("debug").is_table())
        {
            auto debug_table = *build_table.at("debug").as_table();
            load_options(debug_table, this->build.debug.options, config);
            load_define(debug_table, this->build.debug.define, config);
        }
        if (build_table.contains("release") && build_table.at("release").is_table())
        {
            auto release_table = *build_table.at("release").as_table();
            load_options(release_table, this->build.release.options, config);
            load_define(release_table, this->build.release.define, config);
        }
    }
    if (config.table_.contains("dependencies") && config.table_.at("dependencies").is_table())
    {
        auto dependencies = *config.table_.at("dependencies").as_table();
        for (auto &&[name, table] : dependencies)
        {
            auto key = std::string(name.str());
            auto path = parser_env(config.need<std::string>("dependencies." + key + ".path", "", false));
            if (path.empty() && !key.starts_with("Qt"))
                throw std::runtime_error(config.path.string() + ": 'dependencies." + key + "' does not have the 'path'.");
            this->dependencies.insert({key, CupProject{.path = path}});
            if (table.is_table())
            {
                auto sub_table = *table.as_table();
                if (sub_table.contains("features") && sub_table.at("features").is_array())
                {
                    auto defines = *sub_table.at("features").as_array();
                    for (auto &&define : defines)
                    {
                        if (define.is_string())
                        {
                            auto def = define.value<std::string>().value();
                            this->dependencies.at(key).features.push_back(def);
                        }
                        else
                        {
                            throw std::runtime_error(config.path.string() + ": 'dependencies." + key + ".features' must be an array of strings.");
                        }
                    }
                }
            }
            if (key.starts_with("Qt"))
            {
                if (this->qt.has_value())
                    throw std::runtime_error("There are multiple incompatible Qt versions.");
                else
                    this->qt = Qt{.version = key};
                auto features = this->dependencies.at(key).features;
                this->dependencies.erase(key);
                for (auto &&feature : features)
                {
                    std::unordered_set<std::string> flag = {"AUTOMOC", "AUTORCC", "AUTOUIC"};
                    if (flag.find(feature) != flag.end())
                        this->qt.value().flags.push_back(feature);
                    else
                        this->qt.value().modules.push_back(feature);
                }
            }
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
                fs::path dir_str = parser_env(dir.value<std::string>().value());
                this->link.insert({key, dir_str.is_relative() ? config.path.parent_path() / dir_str : dir_str});
            }
            else
            {
                throw std::runtime_error(config.path.string() + ": 'link." + key + "' must be a string.");
            }
        }
    }
}
