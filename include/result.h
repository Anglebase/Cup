#pragma once

#include <variant>
#include <utility>

template <typename T, typename E>
    requires requires(const T &t, const E &e) {
        { T(t) } -> std::same_as<T>;
        { E(e) } -> std::same_as<E>;
    }
class Result
{
    std::variant<T, E> value_;

public:
    bool is_ok() const { return this->value_.index() == 0; }
    bool is_error() const { return this->value_.index() == 1; }

    const T &ok() const { return std::get<0>(this->value_); }
    const E &error() const { return std::get<1>(this->value_); }

    operator bool() const { return this->is_ok(); }
    bool operator!() const { return !this->is_ok(); }

    static Result<T, E> Ok(const T &value)
    {
        Result<T, E> result;
        result.value_ = std::variant<T, E>(std::in_place_index<0>, value);
        return result;
    }
    static Result<T, E> Err(const E &error) {
        Result<T, E> result;
        result.value_ = std::variant<T, E>(std::in_place_index<1>, error);
        return result;
    }
};

template <typename E, typename T>
inline Result<T, E> Ok(const T &value) { return Result<T, E>::Ok(value); }
template <typename T, typename E>
inline Result<T, E> Err(const E &error) { return Result<T, E>::Err(error); }