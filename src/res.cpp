#include "res.h"
#include <fstream>
#include "utils/utils.h"
#include "cmd/git.h"
#include "log.h"

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
    return build(root) / "cmake";
}

fs::path Resource::lib(const fs::path &root)
{
    return target(root) / "lib";
}

fs::path Resource::bin(const fs::path &root)
{
    return target(root) / "bin";
}

fs::path Resource::dll(const fs::path &root)
{
    return target(root) / "dll";
}

fs::path Resource::mod(const fs::path &root)
{
    return target(root) / "mod";
}

fs::path Resource::build(const fs::path &root)
{
    return target(root) / "build";
}

std::pair<fs::path, std::string> Resource::repo_dir(const std::string &url,
     const std::optional<std::string> &version, bool download)
{
    cmd::Git git;
    std::string repo_url;
    if (url.starts_with("@"))
    {
        auto repo_name = split(url.substr(1), "/");
        if (repo_name.size() != 2)
            throw std::runtime_error("Invalid repository name: " + url);
        repo_url = "https://github.com/" + repo_name[0] + "/" + repo_name[1] + ".git";
    }
    else
    {
        repo_url = url;
    }
    auto parts = split(repo_url, "/");
    auto repo_name = parts.back().substr(0, parts.back().find(".git"));
    auto author = parts[parts.size() - 2];
    std::string tag;
    if (version)
        tag = "v" + *version;
    else
    {
        LOG_INFO("Getting latest tag of repository ", repo_url);
        auto tags = git.get_tags(repo_url);
        if (tags.empty())
            throw std::runtime_error("Failed to get tags of repository: " + repo_url);
        tag = tags.back();
        LOG_INFO("Latest tag of repository ", repo_url, " is ", tag);
    }
    auto repo_dir = Resource::packages() / (author + "-" + repo_name + "-" + tag);
    if (!fs::exists(repo_dir) && download)
    {
        LOG_INFO("Cloning repository ", repo_url);
        git.clone(repo_url, repo_dir, tag);
    }
    return {Resource::packages() / (author + "-" + repo_name + "-" + tag), tag.substr(1)};
}
