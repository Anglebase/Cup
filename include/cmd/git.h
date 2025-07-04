#pragma once

#include "cmd/cmd.h"

namespace cmd
{
    /// @brief The Git command class
    class Git
    {
    public:
        Git();

        std::vector<std::string> get_tags(const std::string& url);
        void clone(const std::string& url, const fs::path& dir,const std::string& tag);
    };
}