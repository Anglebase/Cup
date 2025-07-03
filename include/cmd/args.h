#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>

namespace cmd
{
    class Args
    {
        std::vector<std::string> pos_args;
        std::set<std::string> flag_args;
        std::map<std::string, std::vector<std::string>> config_args;

    public:
        Args(int argc, char **argv);

        /// @brief Get the position arguments.
        /// @return The position arguments.
        const std::vector<std::string> &getPositions() const;
        /// @brief  Get the flags.
        /// @return The flags.
        const std::set<std::string> &getFlags() const;
        /// @brief Get the configuration arguments.
        /// @return The configuration arguments.
        const std::map<std::string, std::vector<std::string>> &getConfig() const;

        /// @brief Check if the position argument exists.
        /// @param flag The flag to check.
        /// @return True if the position argument exists, false otherwise.
        bool has_flag(const std::string &flag) const;
        /// @brief Check if the configuration argument exists.
        /// @param config The configuration to check.
        /// @return True if the configuration argument exists, false otherwise.
        bool has_config(const std::string &config) const;

        /// @brief Output the arguments to a stream.
        friend std::ostream &operator<<(std::ostream &os, const Args &args);
    };
}