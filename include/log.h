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
#ifdef _DEBUG
#define LOG_DEBUG(...) _log("Debug", format(__VA_ARGS__)) // 调试级别信息输出
#else
#define LOG_DEBUG(...) // 调试级别信息输出
#endif
#define LOG_INFO(...) _log("Info", format(__VA_ARGS__))                          // 信息级别信息输出
#define LOG_WARN(...) _log("Warn", format("\033[33m", __VA_ARGS__, "\033[0m"))   // 警告级别信息输出
#define LOG_ERROR(...) _log("Error", format("\033[31m", __VA_ARGS__, "\033[0m")) // 错误级别信息输出
