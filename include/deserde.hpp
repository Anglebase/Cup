#pragma once

#include <toml++/toml.hpp>
#include <exception>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>
#include "dollar.h"

class DeserdeError : public std::exception
{
public:
    inline DeserdeError(const std::string &msg = "Inconsistent Type") : m_msg(msg) {}
    inline const char *what() const noexcept override { return m_msg.c_str(); }

private:
    std::string m_msg;
};

template <typename T>
struct Deserde;

template <typename T>
concept Deserdeable = requires(const ::toml::node &node) {
    { Deserde<T>::from(node) } -> std::same_as<T>;
    { Deserde<T>::try_from(node) } -> std::same_as<std::optional<T>>;
};

template <>
struct Deserde<int64_t>
{
    static inline int64_t from(const ::toml::node &node)
    {
        if (node.is_integer())
            return node.value<int64_t>().value();
        throw DeserdeError{};
    }
    static inline std::optional<int64_t> try_from(const ::toml::node &node) noexcept
    {
        return node.value<int64_t>();
    }
};

template <>
struct Deserde<double>
{
    static inline double from(const ::toml::node &node)
    {
        if (node.is_floating_point())
            return node.value<double>().value();
        throw DeserdeError{};
    }
    static inline std::optional<double> try_from(const ::toml::node &node) noexcept
    {
        return node.value<double>();
    }
};

template <>
struct Deserde<bool>
{
    static inline bool from(const ::toml::node &node)
    {
        if (node.is_boolean())
            return node.value<bool>().value();
        throw DeserdeError{};
    }
    static inline std::optional<bool> try_from(const ::toml::node &node) noexcept
    {
        return node.value<bool>();
    }
};

template <>
struct Deserde<std::string>
{
    static inline std::string from(const ::toml::node &node)
    {
        if (node.is_string())
            return Dollar::dollar(node.value<std::string>().value());
        throw DeserdeError{};
    }
    static inline std::optional<std::string> try_from(const ::toml::node &node) noexcept
    {
        return node.value<std::string>().has_value()
                   ? std::optional(Dollar::dollar(node.value<std::string>().value()))
                   : std::nullopt;
    }
};

template <>
struct Deserde<toml::date>
{
    static inline toml::date from(const ::toml::node &node)
    {
        if (node.is_date())
            return node.value<toml::date>().value();
        throw DeserdeError{};
    }
    static inline std::optional<toml::date> try_from(const ::toml::node &node) noexcept
    {
        return node.value<toml::date>();
    }
};

template <>
struct Deserde<toml::time>
{
    static inline toml::time from(const ::toml::node &node)
    {
        if (node.is_time())
            return node.value<toml::time>().value();
        throw DeserdeError{};
    }
    static inline std::optional<toml::time> try_from(const ::toml::node &node) noexcept
    {
        return node.value<toml::time>();
    }
};

template <>
struct Deserde<toml::date_time>
{
    static inline toml::date_time from(const ::toml::node &node)
    {
        if (node.is_date() || node.is_time())
            return node.value<toml::date_time>().value();
        throw DeserdeError{};
    }
    static inline std::optional<toml::date_time> try_from(const ::toml::node &node) noexcept
    {
        return node.value<toml::date_time>();
    }
};

template <typename E>
    requires Deserdeable<E>
struct Deserde<std::vector<E>>
{
    static inline std::vector<E> from(const ::toml::node &node)
    {
        if (!node.is_array())
            throw DeserdeError{};
        std::vector<E> vec;
        for (const auto &item : *node.as_array())
        {
            vec.push_back(Deserde<E>::from(item));
        }
        return vec;
    }
    static inline std::optional<std::vector<E>> try_from(const ::toml::node &node) noexcept
    {
        if (!node.is_array())
            return std::nullopt;
        std::vector<E> vec;
        for (const auto &item : *node.as_array())
        {
            auto val = Deserde<E>::try_from(item);
            if (!val)
                return std::nullopt;
            vec.push_back(val.value());
        }
        return vec;
    }
};

template <typename V>
    requires Deserdeable<V>
struct Deserde<std::map<std::string_view, V>>
{
    static inline std::map<std::string_view, V> from(const ::toml::node &node)
    {
        if (!node.is_table())
            throw DeserdeError{};
        std::map<std::string_view, V> map;
        for (const auto &[key_, value_] : *node.as_table())
        {
            auto key = key_.str();
            auto value = Deserde<V>::from(value_);
            map[std::string_view(key)] = value;
        }
        return map;
    }
    static inline std::optional<std::map<std::string_view, V>> try_from(const ::toml::node &node) noexcept
    {
        if (!node.is_table())
            return std::nullopt;
        std::map<std::string_view, V> map;
        for (const auto &[key_, value_] : *node.as_table())
        {
            auto key = key_.str();
            auto value = Deserde<V>::try_from(value_);
            if (!value)
                return std::nullopt;
            map[std::string_view(key)] = value.value();
        }
        return map;
    }
};

namespace fs = std::filesystem;

template <>
struct Deserde<fs::path>
{
    static inline fs::path from(const ::toml::node &node)
    {
        if (node.is_string())
            return fs::path(Dollar::dollar(node.value<std::string>().value()));
        throw DeserdeError{};
    }
    static inline std::optional<fs::path> try_from(const ::toml::node &node) noexcept
    {
        if (node.is_string())
            return fs::path(Dollar::dollar(node.value<std::string>().value()));
        return std::nullopt;
    }
};

template <typename T, typename U = T>
U require(T *ptr)
{
    if (!ptr)
        throw DeserdeError{};
    return *ptr;
}

template <typename T, typename U = T>
std::optional<U> option(T *ptr)
{
    return ptr == nullptr ? std::nullopt : *ptr;
}

template <typename U>
    requires Deserdeable<U>
U require(const toml::node *node)
{
    if (node == nullptr)
        throw DeserdeError{};
    return Deserde<U>::from(*node);
}

template <typename U>
    requires Deserdeable<U>
std::optional<U> option(const toml::node *node)
{
    if (node == nullptr)
        return std::nullopt;
    return Deserde<U>::try_from(*node);
}