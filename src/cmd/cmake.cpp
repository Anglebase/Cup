#include "cmd/cmake.h"
#include "utils/utils.h"
#include "log.h"

static bool has_cmake()
{
    using namespace cmd;
    Command cmk("cmake");
    cmk.arg("--version");
    auto result = cmk.exec();
    LOG_DEBUG("cmake check:", result.exit_code());
    return result.exit_code() == 0;
}

cmd::CMake::CMake()
{
    static bool has_cmake_ = has_cmake();
    if (!has_cmake_)
    {
        throw std::runtime_error("cmake is not installed.");
    }
    this->commands.push_back("cmake");
}

cmd::CMake &cmd::CMake::source(const fs::path &source_dir)
{
    this->commands.push_back("-S");
    this->commands.push_back(source_dir.string());
    return *this;
}

cmd::CMake &cmd::CMake::build_dir(const fs::path &build_dir)
{
    this->commands.push_back("-B");
    this->commands.push_back(build_dir.string());
    return *this;
}

cmd::CMake &cmd::CMake::generator(const std::string &generator)
{
    this->commands.push_back("-G");
    this->commands.push_back('"' + generator + '"');
    return *this;
}

cmd::CMake &cmd::CMake::target(const std::string &target)
{
    this->commands.push_back("--target");
    this->commands.push_back(target);
    return *this;
}

cmd::CMake &cmd::CMake::build(const fs::path &build_dir)
{
    this->commands.push_back("--build");
    this->commands.push_back(build_dir.string());
    return *this;
}

cmd::CMake &cmd::CMake::config(bool is_release)
{
    this->commands.push_back("--config");
    this->commands.push_back(is_release ? "Release" : "Debug");
    return *this;
}

cmd::CMake &cmd::CMake::jobs(int num_jobs)
{
    this->commands.push_back("-j");
    this->commands.push_back(std::to_string(num_jobs));
    return *this;
}

std::string cmd::CMake::as_command() const
{
    return join(this->commands, " ");
}
