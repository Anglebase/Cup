#pragma once
#include <iostream>

template <typename... Args>
void log_print(Args &&...args) { (std::cout << ... << args) << std::endl; }

#ifdef _DEBUG
#define LOG_DEBUG(...) log_print("\033[32m[DEBUG]", __VA_ARGS__, "\033[0m")
#else
#define LOG_DEBUG(...)
#endif

#define LOG_INFO(...) log_print(__VA_ARGS__)
#define LOG_WARN(...) log_print("\033[33m[WARN]", __VA_ARGS__, "\033[0m")
#define LOG_ERROR(...) log_print("\033[31m[ERROR]", __VA_ARGS__, "\033[0m")