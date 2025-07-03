#pragma once

#include <string>
#include <fstream>

namespace cmd
{
    /// @brief The result of a command execution.
    class Result
    {
        std::string out_;
        std::string err_;
        int exit_code_;

        friend class Command;
        Result(std::string out, std::string err, int exit_code) : out_(out), err_(err), exit_code_(exit_code) {}

    public:
        /// @brief Get the `stdout` of the command result.
        /// @return The `stdout` of the command result.
        const std::string &out() const { return out_; }
        /// @brief Get the `stderr` of the command result.
        /// @return The `stderr` of the command result.
        const std::string &err() const { return err_; }
        /// @brief Get the exit code of the command.
        /// @return The exit code of the command.
        int exit_code() const { return exit_code_; }
    };
}