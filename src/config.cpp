#include "config.h"
#include <fstream>

Config::Config(const fs::path &project)
{
    auto cup_toml = project / "Cup.toml";
    if (!fs::exists(cup_toml))
        throw std::runtime_error("'" + project.string() + "' is not a valid cup project.");
    std::ifstream ifs(cup_toml.string());
    this->table_ = toml::parse(ifs);
    this->path = cup_toml;
}