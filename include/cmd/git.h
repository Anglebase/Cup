#pragma once

#include "cmd/cmd.h"

namespace cmd
{
    /// @brief The Git command class
    class Git
    {
    public:
        Git();

        void clone(const std::string& url, const std::string& dir,const std::string& tag);
    };
}