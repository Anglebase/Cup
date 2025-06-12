#include "dollar.h"
#include <unordered_map>
#include <functional>
#include "log.h"

fs::path Dollar::root = fs::current_path();

std::string Dollar::dollar(const std::string &raw)
{
    if (raw.empty() || !raw.starts_with("$"))
        return raw;

    using Branch = std::function<std::string(const std::string &)>;
    using GotoTable = const std::unordered_map<std::string_view, Branch>;
    static const GotoTable gotoTable = {
        {
            "$root:",
            [&](const std::string &str)
            {
                auto subdir = str.substr(6);
                auto not_1st_slash = subdir.find_first_not_of('/');
                auto not_1st_backslash = subdir.find_first_not_of('\\');
                auto vaild = std::min(subdir.length(), std::max(not_1st_slash, not_1st_backslash));
                subdir = subdir.substr(vaild);
                auto dir = Dollar::root / subdir;
                return dir.lexically_normal().string();
            },
        },
        {
            "$env:",
            [&](const std::string &str)
            {
                auto subdir = str.substr(5);
                auto first_slash = subdir.find_first_of('/');
                auto first_backslash = subdir.find_first_of('\\');
                auto vaild = std::min(first_slash, first_backslash);
                auto env_var = subdir.substr(0, vaild);
                auto env_value = std::getenv(env_var.c_str());
                if (!env_value)
                    throw std::runtime_error("Environment variable not found: " + env_var);
                auto subpath = subdir.substr(vaild);
                auto dir = fs::path(env_value) / subpath;
                return dir.lexically_normal().string();
            },
        },
    };

    for (const auto &[key, value] : gotoTable)
        if (raw.starts_with(key))
        {
            LOG_DEBUG("Goto: ", raw, " -> ", value(raw));
            return value(raw);
        }

    return raw;
}