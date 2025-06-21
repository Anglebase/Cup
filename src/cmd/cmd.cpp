#include "cmd/cmd.h"
#include "utils/utils.h"

Command::Command(std::string execute)
{
    this->args_.push_back(execute);
}

void Command::arg(const std::string &arg)
{
    this->args_.push_back(arg);
}

void Command::set_stdout(const fs::path &path)
{
    this->out = path;
}

void Command::set_stderr(const fs::path &path)
{
    this->err = path;
}

int Command::exec()
{
    if (this->out.has_value())
        this->args("1>", this->out.value().string());
    if (this->err.has_value())
    {
        if (this->out == this->err)
            this->args("2>&1");
        else
            this->args("2>", this->err.value().string());
    }
    auto cmdline = join(this->args_, " ");
    return system(cmdline.c_str());
}
