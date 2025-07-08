#include "cmd/args.h"
#include "subcmd.h"
#include <iostream>
#include <unordered_map>
#include <functional>
#include "log.h"
#include <string>
#include "utils/dollar.h"
using namespace std;

int main(int argc, char **argv)
{
    cmd::Args args(argc, argv);
#ifdef _DEBUG
    cout << args << endl;
#endif
    if (args.getPositions().empty())
    {
        auto help = Help(args);
        return help.run();
    }
    const unordered_map<string, function<int(void)>> subcmd_goto_map = {
        {
            "help",
            [&]()
            {
                auto help = Help(args);
                return help.run();
            },
        },
        {
            "version",
            [&]()
            {
                auto version = Version(args);
                return version.run();
            },
        },
        {
            "new",
            [&]()
            {
                auto new_cmd = New(args);
                return new_cmd.run();
            },
        },
        {
            "build",
            [&]()
            {
                auto build = Build(args);
                return build.run();
            },
        },
        {
            "list",
            [&]()
            {
                auto list = List(args);
                return list.run();
            },
        },
        {
            "install",
            [&]()
            {
                auto install = Install(args);
                return install.run();
            },
        },
        {
            "uninstall",
            [&]()
            {
                auto uninstall = Uninstall(args);
                return uninstall.run();
            },
        },
        {
            "clean",
            [&]()
            {
                auto clean = Clean(args);
                return clean.run();
            },
        },
        {
            "run",
            [&]()
            {
                auto run = Run(args);
                return run.run();
            },
        },
    };
    if (subcmd_goto_map.contains(args.getPositions()[0]))
    {
        try
        {
            return subcmd_goto_map.at(args.getPositions()[0])();
        }
        catch (const exception &e)
        {
            LOG_ERROR(e.what());
            return 1;
        }
    }
    else if (args.getPositions()[0].starts_with("/"))
    {
        try
        {
            auto plugin_name = args.getPositions()[0].substr(1);
            auto plugin = PluginLoader(plugin_name);
            std::optional<std::string> error;
            auto ret = plugin->execute(args, error);
            if (error)
                throw std::runtime_error(*error);
            return ret;
        }
        catch (const exception &e)
        {
            LOG_ERROR(e.what());
        }
    }
    else
    {
        LOG_ERROR("Unknown subcommand: ", args.getPositions()[0]);
        return 1;
    }
    return 0;
}
