#include "git.h"
#include "utils.h"
#include "command.h"
#include <fstream>
#include "log.h"

bool git_valid()
{
    Command git("git");
    git.arg("--version");
    git.set_stdout(get_user_dir() / ".cup" / ".cache");
    return git.execute() == 0;
}

Git::Git()
{
    static bool valid = git_valid();
    if (!valid)
        throw std::runtime_error("Git is not installed or not in PATH.");
}

std::vector<std::string> Git::get_tags(const std::string &url)
{
    Command git("git");
    git.arg("ls-remote");
    git.arg("--tags");
    git.arg(url);
    const auto cache = get_user_dir() / ".cup" / ".cache";
    git.set_stdout(cache);
    auto result = git.execute();
    if (result != 0)
        throw std::runtime_error("Failed to get tags from remote repository.");
    std::ifstream ifs(cache);
    std::string line;
    std::vector<std::string> tags;
    while (std::getline(ifs, line))
    {
        static const std::string prefix = "refs/tags/";
        auto pos = line.find(prefix);
        if (pos == std::string::npos)
            continue;
        auto tag = line.substr(pos + prefix.size());
        tags.push_back(tag);
    }
    return tags;
}

std::pair<std::string, std::string> get_author_libary(const std::string &url)
{
    auto purl = fs::path(url);
    purl.replace_extension();
    std::vector<std::string> parts;
    for (auto part : purl)
    {
        parts.push_back(part.string());
    }
    return {parts[parts.size() - 2], parts[parts.size() - 1]};
}

void Git::download(const std::string &url, const std::string &tag)
{
    const auto &[author, library] = get_author_libary(url);
    const auto cup_dir = get_user_dir() / ".cup";
    const auto dir = cup_dir / author / library / tag;
    Command git("git");
    git.arg("clone");
    git.arg(url);
    git.arg(dir.string());
    git.arg("--branch");
    git.arg(tag);
    git.arg("--depth");
    git.arg("1");
    auto result = git.execute();
    if (result != 0)
        throw std::runtime_error("Failed to download repository.");
    if (!fs::exists(dir / "cup.toml"))
        LOG_WARN("Dependency @", author, "/", library, " (", tag, ") not a cup project.");
}
