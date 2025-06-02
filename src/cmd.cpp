#include "cmd.h"
#include <optional>

SysArgs::SysArgs(int argc, char **argv)
    : args(argv + 1, argv + argc)
{
    auto config = std::optional<std::string>(std::nullopt);
    for (const auto &arg : this->args)
    {
        if (arg.starts_with("--"))
        {
            auto config_ = arg.substr(2);
            config = std::optional<std::string>(config_);
        }
        else if (arg.starts_with("-"))
        {
            auto flag = arg.substr(1);
            this->flags.insert(flag);
        }
        else
        {
            config.has_value()
                ? this->configs[config.value()].push_back(arg)
                : this->arguments.push_back(arg);
        }
    }
}

// 获取命令行输入的位置参数
const std::vector<std::string> &SysArgs::getArguments() const { return this->arguments; }
// 获取命令行输入的选项参数
const std::unordered_set<std::string> &SysArgs::getFlags() const { return this->flags; }
// 获取命令行输入的配置参数
const std::unordered_map<std::string, std::vector<std::string>> &SysArgs::getConfigs() const { return this->configs; }

// 判断是否含有指定的位置参数
bool SysArgs::hasFlag(const std::string &flag) const
{
    return this->flags.find(flag) != this->flags.end();
}

// 判断是否含有指定的配置参数
bool SysArgs::hasConfig(const std::string &config) const
{
    return this->configs.find(config) != this->configs.end();
}