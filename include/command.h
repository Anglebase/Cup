#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
namespace fs = std::filesystem;

class Command
{
    std::vector<std::string> args;
    std::optional<fs::path> stdout_;
    std::optional<fs::path> stderr_;

public:
    Command(const std::string &name);
    Command &arg(const std::string &arg);

    Command &set_stdout(const fs::path &stdout_);
    Command &set_stderr(const fs::path &stderr_);

    int execute();
};