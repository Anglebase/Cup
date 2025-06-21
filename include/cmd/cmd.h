#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <type_traits>
namespace fs = std::filesystem;

class Command
{
    std::vector<std::string> args_;
    std::optional<fs::path> out;
    std::optional<fs::path> err;

public:
    Command(std::string execute);
    void arg(const std::string &arg);

    void set_stdout(const fs::path &path);
    void set_stderr(const fs::path &path);

    template <class... Args>
        requires(std::is_convertible_v<Args, std::string> && ...)
    void args(Args &&...args)
    {
        (args_.emplace_back(std::forward<Args>(args)), ...);
    }

    int exec();
};