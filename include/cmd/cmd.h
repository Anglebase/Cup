#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <type_traits>
#include "cmd/result.h"
namespace fs = std::filesystem;

namespace cmd
{
    /// @brief 命令行程序执行类
    class Command
    {
        std::vector<std::string> args_;
        std::optional<fs::path> out;
        std::optional<fs::path> err;

    public:
        Command(std::string execute);
        /// @brief 设置命令行程序标准输出重定向路径
        /// @param path 标准输出重定向路径
        void set_stdout(const fs::path &path);
        /// @brief 设置命令行程序标准错误重定向路径
        /// @param path 标准错误重定向路径
        void set_stderr(const fs::path &path);
        /// @brief 添加命令行参数
        /// @param arg 命令行参数
        void arg(const std::string &arg);

        /// @brief 设置命令行参数
        /// @param ...args 可以有任意个参数，所有参数必须能够转换为std::string类型
        template <class... Args>
            requires(std::is_convertible_v<Args, std::string> && ...)
        void args(Args &&...args)
        {
            (args_.emplace_back(std::forward<Args>(args)), ...);
        }

        /// @brief 执行命令行并返回结果
        /// @return 包含命令行程序的标准输出、标准错误、退出状态码的对象
        Result exec();
        /// @brief 执行命令行代码并返回退出状态码
        /// @return 命令行的退出状态码
        int run();
    };
}