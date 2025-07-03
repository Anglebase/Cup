#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

/// @brief 字符串分割
/// @param str 原始字符串
/// @param delimiter 分隔符
/// @return 分割后的字符串数组
std::vector<std::string> split(const std::string &str, const std::string &delimiter);
/// @brief 字符串连接
/// @param strs 字符串数组
/// @param delimiter 分隔符
/// @return 连接后的字符串
std::string join(const std::vector<std::string> &strs, const std::string &delimiter);
/// @brief 一次性读取文件内容
/// @param file_path 文件路径
/// @return 读取的文件内容
std::string read_file(const fs::path &file_path);
/// @brief 字符串替换
/// @param str 原始字符串
/// @param old_str 旧字符串
/// @param new_str 新字符串
/// @return 替换后的字符串
std::string replace(const std::string &str, const std::string &old_str = "\\", const std::string &new_str = "/");

template <typename Iter, typename F>
std::string join(Iter begin, Iter end, const std::string &delimiter, F &&f)
{
    std::string result;
    for (auto it = begin; it != end; ++it)
    {
        if (!result.empty())
            result += delimiter;
        result += f(*it);
    }
    return result;
}