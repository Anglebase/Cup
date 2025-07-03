#include "utils/dollar.h"

fs::path Dollar::ROOT = fs::current_path();

std::string Dollar::dollar(const std::string &str)
{
    if (!str.starts_with("${"))
        return str;
    auto pos = str.find("}");
    if (pos == std::string::npos)
        return str;
    auto substr = str.substr(pos + 1);
    auto var_name = str.substr(2, pos - 2);
    if (var_name == "root")
    {
        return (ROOT / substr).lexically_normal().string();
    }
    else if (var_name.starts_with("env:"))
    {
        auto env_name = var_name.substr(4);
        auto env_value = std::getenv(env_name.c_str());
        if (env_value == nullptr)
            throw std::runtime_error("Environment variable not found: " + env_name);
        return std::string(env_value) + substr;
    }
    throw std::runtime_error("Invalid dollar variable name: " + var_name);
}