#pragma once

#include <filesystem>
#include <string>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace plugin
{
    /// @brief Dynamic library loader
    class Loader
    {
#ifdef _WIN32
        HMODULE handle_;
#else
        void *handle_;
#endif

    public:
        /// @brief 构造函数
        /// @param path 动态库路径
        Loader(const fs::path &path);
        ~Loader();

        /// @brief 获取动态库中的符号
        /// @param name 符号名
        /// @return 符号指针
        void *get_symbol(const std::string &name);
    };
}