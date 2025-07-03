#include "res.h"
#include <fstream>

fs::path Resource::home()
{
    static auto home = std::getenv(
#ifdef _WIN32
        "USERPROFILE"
#else
        "HOME"
#endif
    );
    if (home == nullptr)
    {
        throw std::runtime_error("Failed to get home directory");
    }
    return fs::path(home);
}

fs::path Resource::cup()
{
    static auto cup = home() / ".cup";
    return cup;
}

fs::path Resource::cache()
{
    static auto cache = cup() / "cache";
    return cache;
}

fs::path Resource::plugins()
{
    static auto plugins = cup() / "plugins";
    return plugins;
}

fs::path Resource::packages()
{
    static auto packages = cup() / "packages";
    return packages;
}

std::string Resource::read_cache(const std::string &file_name)
{
    std::string result;
    auto cache_file = Resource::cache() / file_name;
    std::ifstream file(cache_file);
    if (!file.is_open())
        return result;
    std::string buffer;
    while (std::getline(file, buffer))
        result += buffer + "\n";
    return result;
}

fs::path Resource::target(const fs::path &root)
{
    return root / "target";
}

fs::path Resource::cmake(const fs::path &root)
{
    return target(root) / "cmake";
}

fs::path Resource::lib(const fs::path &root)
{
    return target(root) / "lib";
}

fs::path Resource::bin(const fs::path &root)
{
    return target(root) / "bin";
}

fs::path Resource::build(const fs::path &root)
{
    return target(root) / "build";
}
