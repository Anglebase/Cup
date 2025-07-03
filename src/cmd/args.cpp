#include "cmd/args.h"
#include <optional>

cmd::Args::Args(int argc, char **argv)
{
    auto args = std::vector<std::string>(argv + 1, argv + argc);
    auto config = std::optional<std::string>(std::nullopt);
    for (const auto &arg : args)
    {
        if (arg.starts_with("--"))
        {
            auto config_ = arg.substr(2);
            config = std::optional<std::string>(config_);
        }
        else if (arg.starts_with("-"))
        {
            auto flag = arg.substr(1);
            this->flag_args.insert(flag);
        }
        else
        {
            config.has_value()
                ? this->config_args[config.value()].push_back(arg)
                : this->pos_args.push_back(arg);
        }
    }
}

const std::vector<std::string> &cmd::Args::getPositions() const
{
    return this->pos_args;
}

const std::set<std::string> &cmd::Args::getFlags() const
{
    return this->flag_args;
}

const std::map<std::string, std::vector<std::string>> &cmd::Args::getConfig() const
{
    return this->config_args;
}

bool cmd::Args::has_flag(const std::string &flag) const
{
    return this->flag_args.find(flag) != this->flag_args.end();
}

bool cmd::Args::has_config(const std::string &config) const
{
    return this->config_args.find(config) != this->config_args.end();
}

std::ostream &cmd::operator<<(std::ostream &os, const Args &args)
{
    os << "Positional arguments: " << std::endl;
    for (const auto &arg : args.pos_args)
        os << arg << std::endl;
    os << "Flag arguments: " << std::endl;
    for (const auto &flag : args.flag_args)
        os << flag << std::endl;
    os << "Config arguments: " << std::endl;
    for (const auto &[key, value] : args.config_args)
    {
        os << key << ": " << std::endl;
        for (const auto &value : value)
            os << value << std::endl;
    }
    return os;
}
