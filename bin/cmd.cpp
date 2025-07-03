#include "cmd.h"
#include <iostream>

int main(int argc, char **argv)
{
    auto args = SysArgs(argc, argv);
    std::cout << "Arguments: ";
    for (const auto &arg : args.getArguments())
        std::cout << arg << " ";
    std::cout << std::endl;
    std::cout << "Flags: ";
    for (const auto &flag : args.getFlags())
        std::cout << flag << " ";
    std::cout << std::endl;
    std::cout << "Configs: " << std::endl;
    for (const auto &[key, value] : args.getConfigs())
    {
        std::cout << key << " = ";
        for (const auto &v : value)
            std::cout << v << " ";
        std::cout << std::endl;
    }
    return 0;
}