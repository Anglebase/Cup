#include "command.h"
#include "utils.h"
#include "log.h"

Command::Command(const std::string &name)
    : args({name})
{
}

Command &Command::arg(const std::string &arg)
{
    this->args.push_back(arg);
    return *this;
}

Command &Command::set_stdout(const fs::path &stdout_)
{
    this->stdout_ = stdout_;
    return *this;
}

Command &Command::set_stderr(const fs::path &stderr_)
{
    this->stderr_ = stderr_;
    return *this;
}

int Command::execute()
{
    if (this->stdout_)
        this->args.push_back("1>\"" + this->stdout_->string() + "\"");
    if (this->stderr_)
        this->args.push_back("2>\"" + this->stderr_->string() + "\"");
    auto cmd = join(this->args, " ");
    LOG_DEBUG("Run: ", cmd);
    auto result = system(cmd.c_str());
    return result;
}
