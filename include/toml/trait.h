#pragma once

#include "toml++/toml.hpp"
#include <string>
#include <vector>
#include <map>
#include <optional>

namespace data
{
    /// @brief TOML 反序列化 trait
    /// @tparam T 被反序列化的类型
    template <typename T>
    struct Deserializer
    {
        static inline T deserialize(const toml::node &v, const std::string &key = "");
    };

    /// @brief 类型 T 是否可反序列化
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
    TOML_DESERIALIZE(String);
    TOML_DESERIALIZE(Time);
    TOML_DESERIALIZE(Date);
    TOML_DESERIALIZE(DateTime);

#undef TOML_DESERIALIZE

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
                result[k] = Deserializer<V>::deserialize(v, key + "." + k);
            return result;
        }
    };

    template <Deserializable E>
    using Array = std::vector<E>;
    template <Deserializable V>
    using Table = std::map<std::string, V>;
    template <Deserializable T>
    using Optional = std::optional<T>;

    /// @brief 要求必须反序列化一个 TOML 值
    /// @tparam T 被反序列化到的类型
    /// @param table TOML 表
    /// @param key 表的键
    /// @param value 反序列化得到的结果
    /// @param key_desc 表的访问路径
    template <Deserializable T>
    void require(const toml::table &table, const std::string &key, T &value, const std::string &key_desc = "")
    {
        value = Deserializer<T>::deserialize(table.at(key), key_desc);
    }

    /// @brief 要求可选反序列化一个 TOML 值
    /// @tparam T 被反序列化到的类型
    /// @param table TOML 表
    /// @param key 表的键
    /// @param value 反序列化得到的结果
    /// @param key_desc 表的访问路径
    template <Deserializable T>
    void options(const toml::table &table, const std::string &key, Optional<T> &value, const std::string &key_desc = "")
    {
        table.contains(key)
            ? value = Deserializer<T>::deserialize(table.at(key), key_desc)
            : value = std::nullopt;
    }
}