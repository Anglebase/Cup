#pragma once

#include "cup_plugin/interface.h"
#include "utils/utils.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

inline void _cycle_dep_check(const std::string &key, const std::map<std::string, std::vector<std::string>> &table)
{
    static std::vector<std::string> cycle_check;
    auto has_cycle = std::find(cycle_check.begin(), cycle_check.end(), key) != cycle_check.end();
    cycle_check.push_back(key);
    if (has_cycle)
    {
        std::string cycle_str = join(cycle_check, " -> ");
        throw std::runtime_error("Features cycle dependency detected: " + cycle_str);
    }
    if (!table.contains(key))
        throw std::runtime_error("Feature '" + key + "' not found in [features]");
    for (const auto &value : table.at(key))
        _cycle_dep_check(value, table);
    cycle_check.pop_back();
}

inline void cycle_dep_check(const std::optional<std::map<std::string, std::vector<std::string>>> &table)
{
    if (!table)
        return;
    for (const auto &[key, _] : *table)
        _cycle_dep_check(key, *table);
}

inline std::vector<std::string> get_features(
    const std::optional<std::vector<std::string>> &features,
    const std::optional<std::map<std::string, std::vector<std::string>>> &table)
{
    if (!features)
        return std::vector<std::string>();
    auto buffer = *features;
    cycle_dep_check(table);
    if (!table)
        return buffer;
    std::vector<std::string> result;
    bool contains = false;
    do
    {
        contains = false;
        for (const auto &[key, value] : *table)
        {
            auto iter = std::find(buffer.begin(), buffer.end(), key);
            if (iter != buffer.end())
            {
                contains = true;
                if (std::find(result.begin(), result.end(), *iter) == result.end())
                    result.push_back(*iter);
                buffer.erase(iter);
                buffer.insert(buffer.end(), value.begin(), value.end());
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
        {
            prefix + "SOURCES",
            config && config->sources
                ? join(*config->sources, " ", [](const fs::path &p)
                       { return '"' + replace(p.string()) + '"'; })
                : "",
        },
        {
            prefix + "COMPILER_FEAT",
            config && config->compiler_features ? join(*config->compiler_features, " ") : "",
        },
    };
}

inline std::string dealpath(const fs::path &p)
{
    return '"' + replace(p.string()) + '"';
}

inline std::unordered_map<std::string, std::string> gen_feat_replacement(const std::vector<std::string> &name)
{
    static const std::vector<std::string> suffix = {
        "_INCLUDE_DIRS",
        "_LIB_DIRS",
        "_LIBS",
        "_DEFINES",
        "_COPTIONS",
        "_LINKOPTIONS",
        "_SOURCES",
        "_COMPILER_FEAT",
    };
    std::unordered_map<std::string, std::string> result;
    for (const auto &s : suffix)
    {
        auto key = "FEAT" + s;
        result[key] = "";
        for (const auto &n : name)
            result[key] += "${FEAT_" + n + s + "} ${FEAT_" + n + "_MODE" + s + "}";
    }
    return result;
}

inline bool is_source_file(const fs::path &p)
{
    static const std::unordered_set<std::string> suffix = {
        ".c",
        ".C",
        ".c++",
        ".cc",
        ".cpp",
        ".cxx",
        ".cu",
        ".mpp",
        ".m",
        ".M",
        ".mm",
        ".ixx",
        ".cppm",
        ".ccm",
        ".cxxm",
        ".c++m",
        ".h",
        ".hh",
        ".h++",
        ".hm",
        ".hpp",
        ".hxx",
        ".in",
        ".txx",
        ".f",
        ".F",
        ".for",
        ".f77",
        ".f90",
        ".f95",
        ".f03",
        ".hip",
        ".ispc",
    };
    return suffix.contains(p.extension().string());
}