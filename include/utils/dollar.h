#pragma once

#include <string>
#include <filesystem>
#include <cassert>
namespace fs = std::filesystem;

struct Dollar
{
    static fs::path ROOT;

    static std::string dollar(const std::string &str);
};