#pragma once

#include "plugin/interface.h"
#include "utils/utils.h"

inline std::vector<std::string> get_features(
    const std::optional<std::vector<std::string>> &features,
    const std::optional<std::map<std::string, std::vector<std::string>>> &table)
{
    if (!features)
        return std::vector<std::string>();
    auto result = *features;
    if (!table)
        return result;
    std::vector<std::string> buffer;
    bool contains = false;
    do
    {
        contains = false;
        for (const auto &[key, value] : *table)
        {
            auto iter = std::find(result.begin(), result.end(), key);
            if (iter != result.end())
            {
                contains = true;
                if (std::find(buffer.begin(), buffer.end(), *iter) == buffer.end())
                    buffer.push_back(*iter);
                result.erase(iter);
                result.insert(result.end(), value.begin(), value.end());
            }
        }
    } while (contains);
    return result;
}


template <class T>
std::unordered_map<std::string, std::string> gen_map(const std::string &prefix, const std::optional<T> &config)
{
    return {
        {
            prefix + "INCLUDE_DIRS",
            config && config->includes
                ? join(*config->includes, " ", [](const fs::path &p)
                       { return '"' + replace(p.string()) + '"'; })
                : "",
        },
        {
            prefix + "LIB_DIRS",
            config && config->link_dirs
                ? join(*config->link_dirs, " ", [](const fs::path &p)
                       { return '"' + replace(p.string()) + '"'; })
                : "",
        },
        {
            prefix + "LIBS",
            config && config->link_libs ? join(*config->link_libs, " ") : "",
        },
        {
            prefix + "DEFINES",
            config && config->defines
                ? join(*config->defines, " ", [](const std::string &s)
                       { return "-D" + s; })
                : "",
        },
        {
            prefix + "COPTIONS",
            config && config->compile_options ? join(*config->compile_options, " ") : "",
        },
        {
            prefix + "LINKOPTIONS",
            config && config->link_options ? join(*config->link_options, " ") : "",
        },
    };
}

inline std::string dealpath(const fs::path &p)
{
    return '"' + replace(p.string()) + '"';
}