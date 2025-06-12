#include "config.h"
#include <fstream>
#include <unordered_set>

Config::Config(const fs::path &project)
{
    auto cup_toml = project / "cup.toml";
    if (!fs::exists(cup_toml))
        throw std::runtime_error("'" + project.lexically_normal().string() + "' is not a valid cup project.");
    std::ifstream ifs(cup_toml);
    this->table_ = toml::parse(ifs);
    this->path = cup_toml;
    Dollar::root = project;
    this->config = new ConfigInfo(*this);
}

Config::~Config()
{
    delete this->config;
}

fs::path parser_env(const std::string &str, const fs::path &root)
{
    if (str.empty())
        return fs::path();
    if (str.starts_with("$root:"))
    {
        LOG_DEBUG("$root:", root);
        LOG_DEBUG("sub: ", str.substr(6));
        LOG_DEBUG("New Dir: ", root / str.substr(6));
        return root / str.substr(6);
    }
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