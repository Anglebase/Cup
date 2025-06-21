#pragma once

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

class Resource
{
public:
    static fs::path home();
    static fs::path cache();
    static std::string read_cache();
};