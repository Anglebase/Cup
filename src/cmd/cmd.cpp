#include "cmd/cmd.h"
#include "utils/utils.h"
#include "res/res.h"
#include "cmd.h"
namespace cmd
{
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

    Result Command::exec()
    {
        const auto outpath = Resource::home() / "out.cache";
        const auto errpath = Resource::home() / "err.cache";
        this->set_stdout(outpath);
        this->set_stderr(errpath);

        auto ret = this->run();
        auto errmsg = read_file(errpath);
        auto outmsg = read_file(outpath);
        return Result(outmsg, errmsg, ret);
    }

    int Command::run()
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
}