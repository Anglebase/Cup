#include "cmd/git.h"
#include "utils/utils.h"

bool has_git()
{
    using namespace cmd;
    Command cmd("git");
    cmd.arg("--version");
    auto result = cmd.exec();
    return result.exit_code() == 0;
}

cmd::Git::Git()
{
    static bool has_git_ = has_git();
    if (!has_git_)
        throw std::runtime_error("git is not installed.");
}
std::vector<std::string> cmd::Git::get_tags(const std::string &url)
{
    using namespace cmd;
    Command cmd("git");
    cmd.args("ls-remote", "--tags", url);
    auto result = cmd.exec();
    if (result.exit_code() != 0)
        throw std::runtime_error("Failed to get tags from repository.");
    std::vector<std::string> tags;
    for (auto line : split(result.out(), "\n"))
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

void cmd::Git::clone(const std::string &url, const fs::path &dir, const std::string &tag)
{
    using namespace cmd;
    Command cmd("git");
    cmd.args("clone", url, dir.lexically_normal().string(), "--branch", tag, "--depth", "1");
    auto res = cmd.run();
    if (res != 0)
        throw std::runtime_error("Failed to clone repository.");
}
