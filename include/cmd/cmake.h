#pragma once

#include "cmd/cmd.h"
#include <thread>

namespace cmd
{
    /// @brief The CMake command class.
    class CMake
    {
        std::vector<std::string> commands;

    public:
        CMake();

        CMake &source(const fs::path &source_dir);
        CMake &build_dir(const fs::path &build_dir);
        CMake &generator(const std::string &generator);

        CMake &target(const std::string &target);
        CMake &build(const fs::path &build_dir);
        CMake &config(bool is_release = false);

        CMake &jobs(int num_jobs = std::thread::hardware_concurrency());

        std::string as_command() const;
    };
}