#pragma once

#include <string>
#include <fstream>

namespace cmd
{
    class Result
    {
        std::string out_;
        std::string err_;
        int exit_code_;

        friend class Command;
        Result(std::string out, std::string err, int exit_code) : out_(out), err_(err), exit_code_(exit_code) {}

    public:
        const std::string &out() const { return out_; }
        const std::string &err() const { return err_; }
        int exit_code() const { return exit_code_; }
    };
}