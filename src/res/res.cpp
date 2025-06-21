#include "res/res.h"
#include <fstream>
#include <vector>
#include "utils/utils.h"

fs::path Resource::home()
{
#ifdef _WIN32
#define HOME_ENV "USERPROFILE"
#else
#define HOME_ENV "HOME"
#endif
    static const fs::path home_path =
        fs::path(getenv(HOME_ENV)) /
        ".cup";
    return home_path;
#undef HOME_ENV
}

fs::path Resource::cache()
{
    static const fs::path cache_path = home() / "cache";
    return cache_path;
}

std::string Resource::read_cache()
{
    std::vector<std::string> lines;
    std::ifstream file(Resource::cache());
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();
    }
    else
    {
        throw std::runtime_error("Failed to open cache file.");
    }
    return join(lines, "\n");
}
