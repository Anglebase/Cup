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
    class Command
    {
        std::vector<std::string> args_;
        std::optional<fs::path> out;
        std::optional<fs::path> err;

    public:
        Command(std::string execute);
        /// @brief set the `stdout` redirection path of the command
        /// @param path `stdout` redirection path
        void set_stdout(const fs::path &path);
        /// @brief set the `stderr` redirection path of the command
        /// @param path `stderr` redirection path
        void set_stderr(const fs::path &path);
        /// @brief add a new argument to the command line
        /// @param arg argument
        void arg(const std::string &arg);

        /// @brief add new arguments to the command line
        /// @param ...args arguments
        template <class... Args>
            requires(std::is_convertible_v<Args, std::string> && ...)
        void args(Args &&...args)
        {
            (args_.emplace_back(std::forward<Args>(args)), ...);
        }

        /// @brief Execute the command line program and return the result
        /// @return the result of the command line
        /// @note The redirection of `stdout` and `stderr` will be ignored.
        ///       It will return the content of the `stdout` and `stderr` files.
        Result exec();
        /// @brief Run the command line program and return the exit status code
        /// @return the exit status code of the command line
        /// @note The output of the command line will be collected and stored in the `stdout` and `stderr` files.
        ///       If not specified, the `stdout` and `stderr` will be output to the console.
        int run();
    };
}