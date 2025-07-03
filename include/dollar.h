#pragma once

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

struct Dollar
{
    static fs::path root;
    static std::string dollar(const std::string& raw);
};