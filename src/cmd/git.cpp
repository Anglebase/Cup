#include "cmd/git.h"

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

void cmd::Git::clone(const std::string &url, const std::string &dir, const std::string &branch)
{
    using namespace cmd;
    Command cmd("git");
    cmd.args("clone", url, dir, "--branch", branch, "--depth", "1");
    auto res = cmd.run();
    if (res != 0)
        throw std::runtime_error("Failed to clone repository.");
}
