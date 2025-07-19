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
    Result(const T &value) : value_(std::in_place_index<0>, value) {}
    Result(const E &error) : value_(std::in_place_index<1>, error) {}

    template <typename U>
        requires std::convertible_to<U, T>
    Result(const U &other) : value_(std::in_place_index<0>, other)
    {
    }
    template <typename U>
        requires std::convertible_to<U, E>
    Result(const U &other) : value_(std::in_place_index<1>, other)
    {
    }

    bool is_ok() const { return this->value_.index() == 0; }
    bool is_error() const { return this->value_.index() == 1; }

    const T &ok() const { return std::get<0>(this->value_); }
    const E &error() const { return std::get<1>(this->value_); }

    operator bool() const { return this->is_ok(); }
    bool operator!() const { return !this->is_ok(); }

    static Result<T, E> Ok(const T &value) { return Result<T, E>(std::in_place_index<0>, value); }
    static Result<T, E> Err(const E &error) { return Result<T, E>(std::in_place_index<1>, error); }
};

template <typename E, typename T>
inline Result<T, E> Ok(const T &value) { return Result<T, E>::Ok(value); }
template <typename T, typename E>
inline Result<T, E> Err(const E &error) { return Result<T, E>::Err(error); }