#pragma once

#include <sstream>
#include <iostream>

inline void _log(const std::string &type, const std::string &log)
{
    std::cout << "[" << type << "] " << log << std::endl;
}

template <class... Args>
inline std::string format(Args... args)
{
    std::ostringstream oss;
    (oss << ... << args);
    return oss.str();
}

#define LOG_DEBUG(...) _log("Debug", format(__VA_ARGS__))
#define LOG_INFO(...) _log("Info ", format(__VA_ARGS__))
#define LOG_WARN(...) _log("Warn ", format(__VA_ARGS__))
#define LOG_ERROR(...) _log("Error", format(__VA_ARGS__))
