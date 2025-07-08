#pragma once

#include "toml++/toml.hpp"
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>
#include "utils/dollar.h"
namespace fs = std::filesystem;

namespace data
{
    template <typename T>
    struct Deserializer
    {
        static inline T deserialize(const toml::node &v, const std::string &key = "");
    };

    template <typename T>
    concept Deserializable = requires(const toml::node &v) {
        { Deserializer<T>::deserialize(v, "") } -> std::same_as<T>;
    };

#define TOML_DESERIALIZE(T)                                                           \
    template <>                                                                       \
    struct Deserializer<T>                                                            \
    {                                                                                 \
        static inline T deserialize(const toml::node &v, const std::string &key = "") \
        {                                                                             \
            if (!v.is<T>())                                                           \
                throw std::runtime_error("toml value for " + key + " is not a " #T);  \
            return *v.value<T>();                                                     \
        }                                                                             \
    }

    using Boolean = bool;
    using Integer = int64_t;
    using Float = double;
    using String = std::string;
    using Time = toml::time;
    using Date = toml::date;
    using DateTime = toml::date_time;

    TOML_DESERIALIZE(Boolean);
    TOML_DESERIALIZE(Integer);
    TOML_DESERIALIZE(Float);
    TOML_DESERIALIZE(Time);
    TOML_DESERIALIZE(Date);
    TOML_DESERIALIZE(DateTime);

#undef TOML_DESERIALIZE

    template <>
    struct Deserializer<String>
    {
        static inline String deserialize(const toml::node &v, const std::string &key = "")
        {
            if (!v.is_string())
                throw std::runtime_error("toml value for " + key + " is not a string");
            return Dollar::dollar(*v.value<std::string>());
        }
    };

    template <>
    struct Deserializer<fs::path>
    {
        static inline fs::path deserialize(const toml::node &v, const std::string &key = "")
        {
            if (!v.is_string())
                throw std::runtime_error("toml value for " + key + " is not a string");
            fs::path path = Dollar::dollar(*v.value<std::string>());
            if (path.is_relative())
                path = Dollar::ROOT / path;
            return path;
        }
    };

    template <Deserializable E>
    struct Deserializer<std::vector<E>>
    {
        static inline std::vector<E> deserialize(const toml::node &v, const std::string &key = "")
        {
            if (!v.is_array())
                throw std::runtime_error("toml value for " + key + " is not an array");
            auto array = *v.as_array();
            std::vector<E> result;
            for (size_t i = 0; i < array.size(); ++i)
                result.push_back(Deserializer<E>::deserialize(array.at(i), key + "[" + std::to_string(i) + "]"));
            return result;
        }
    };

    template <Deserializable V>
    struct Deserializer<std::map<std::string, V>>
    {
        static inline std::map<std::string, V> deserialize(const toml::node &v, const std::string &key = "")
        {
            if (!v.is_table())
                throw std::runtime_error("toml value for " + key + " is not a table");
            std::map<std::string, V> result;
            for (const auto &[k, v] : *v.as_table())
                result[std::string(k.str())] = Deserializer<V>::deserialize(v, key + "." + std::string(k.str()));
            return result;
        }
    };

    template <Deserializable E>
    using Array = std::vector<E>;
    template <Deserializable V>
    using Table = std::map<std::string, V>;
    template <Deserializable T>
    using Optional = std::optional<T>;

    template <Deserializable T>
    void require(const toml::table &table, const std::string &key, T &value, const std::string &key_desc = "")
    {
        value = Deserializer<T>::deserialize(table.at(key), key_desc);
    }

    template <Deserializable T>
    void options(const toml::table &table, const std::string &key, Optional<T> &value, const std::string &key_desc = "")
    {
        table.contains(key)
            ? value = Deserializer<T>::deserialize(table.at(key), key_desc)
            : value = std::nullopt;
    }

    template <Deserializable T>
    T parse_toml_file(const fs::path &path)
    {
        Dollar::ROOT = path.parent_path();
        return Deserializer<T>::deserialize(toml::parse_file(path.string()));
    }
}

#define TOML_DESERIALIZE(T, MEMBER)                                                   \
    template <>                                                                       \
    struct Deserializer<T>                                                            \
    {                                                                                 \
        static inline T deserialize(const toml::node &v, const std::string &key = "") \
        {                                                                             \
            if (!v.is_table())                                                        \
                throw std::runtime_error(key + " is not a table");                    \
            auto table = *v.as_table();                                               \
            T result;                                                                 \
            MEMBER return result;                                                     \
        }                                                                             \
    }

#define TOML_REQUIRE(k) require(table, #k, result.k, key)
#define TOML_OPTIONS(k) options(table, #k, result.k, key)