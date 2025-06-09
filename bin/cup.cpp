#include <iostream>
#include "cmd.h"
#include "log.h"
#include <unordered_map>
#include <functional>
#include "tools.h"

int main(int argc, char **argv)
{
    const auto args = SysArgs(argc, argv);
#ifdef _DEBUG
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
#endif
    if (args.getArguments().empty())
    {
        LOG_DEBUG("Help| Error: {No Cmd}");
        return 0;
    }
    auto cmd = args.getArguments()[0];
    std::unordered_map<std::string, std::function<int(void)>> table = {
        {"new", [&]()
         {
             NewCmd newCmd(args);
             return newCmd.run();
         }},
        {"clean", [&]()
         {
             CleanCmd cleanCmd(args);
             return cleanCmd.run();
         }},
        {"build", [&]()
         {
             BuildCmd buildCmd(args);
             return buildCmd.run();
         }},
        {"run", [&]()
         {
             RunCmd runCmd(args);
             return runCmd.exec();
         }},
        {"list", [&]()
         {
             ListCmd listCmd(args);
             return listCmd.run();
         }},
    };
    if (table.find(cmd) == table.end())
    {
        LOG_DEBUG("Help| Error: {No Cmd}");
        return 0;
    }
    try
    {
        return table[cmd]();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Exception: ", e.what());
        return 1;
    }
}
