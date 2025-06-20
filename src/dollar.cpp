#include "dollar.h"
#include <unordered_map>
#include <functional>
#include "log.h"
#include "tools.h"

fs::path Dollar::root = fs::current_path();

std::string Dollar::dollar(const std::string &raw)
{
    if (raw.empty() || !raw.starts_with("$"))
        return raw;

    using Branch = std::function<std::string(const std::string &)>;
    using GotoTable = const std::unordered_map<std::string_view, Branch>;
    static const GotoTable gotoTable = {
        {
            "${root}",
            [&](const std::string &str)
            {
                auto subdir = str.substr(7);
                auto not_1st_slash = subdir.find_first_not_of('/');
                auto not_1st_backslash = subdir.find_first_not_of('\\');
                auto vaild = std::min(subdir.length(), std::max(not_1st_slash, not_1st_backslash));
                subdir = subdir.substr(vaild);
                auto dir = Dollar::root / subdir;
                return dir.lexically_normal().string();
            },
        },
        {
            "${env:",
            [&](const std::string &str)
            {
                auto subdir = str.substr(6);
                auto end = subdir.find_first_of('}');
                if (end == std::string::npos)
                    throw std::runtime_error("Invalid replacement sequence: " + str);
                auto env_var = subdir.substr(0, end);
                auto env_value = std::getenv(env_var.c_str());
                if (!env_value)
                    throw std::runtime_error("Environment variable not found: " + env_var);
                auto subpath = subdir.substr(end + 1);
                auto dir = fs::path(env_value) / subpath;
                return dir.lexically_normal().string();
            },
        },
    };

    for (const auto &[key, value] : gotoTable)
        if (raw.starts_with(key))
        {
            LOG_DEBUG("Goto: ", raw, " -> ", value(raw));
            return replace_all(value(raw), "\\", "/");
        }

    return raw;
}