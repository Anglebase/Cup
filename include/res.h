#pragma once

#include <string>
#include <filesystem>
#include <optional>
namespace fs = std::filesystem;

/// @brief A class to manage resources.
class Resource
{
public:
    /// @brief Get the home directory of the user.
    /// @return The home directory of the user.
    static fs::path home();
    /// @brief Get the CUP directory of the user.
    /// @return The CUP directory of the user.
    static fs::path cup();
    /// @brief Get the cache directory of the user.
    /// @return The cache directory of the user.
    static fs::path cache();
    /// @brief Get the plugins directory of the user.
    /// @return The plugins directory of the user.
    static fs::path plugins();
    /// @brief Get the packages directory of the user.
    /// @return The packages directory of the user.
    static fs::path packages();
    /// @brief Get the content of the cache file.
    /// @param file_name The name of the cache file.
    /// @return The content of the cache file.
    static std::string read_cache(const std::string& file_name);

    static fs::path target(const fs::path& root);
    static fs::path cmake(const fs::path& root);
    static fs::path lib(const fs::path& root);
    static fs::path bin(const fs::path& root);
    static fs::path dll(const fs::path& root);
    static fs::path mod(const fs::path& root);
    static fs::path build(const fs::path& root);
    static std::pair<fs::path, std::string> repo_dir(const std::string& url,
         const std::optional<std::string>& version, bool download = true);
};