#include "cup_plugin/args.h"
#include "subcmd.h"
#include <iostream>
#include <unordered_map>
#include <functional>
#include "log.h"
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    cmd::Args args(argc, argv);
#ifdef _DEBUG
    cout << args << endl;
#endif
    if (args.has_flag("version") || args.has_flag("v") || args.has_config("version") || args.has_config("v"))
    {
        auto version = Version(args);
        return version.run();
    }
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
            auto ret = plugin->execute(args);
            if (ret.is_error())
                throw std::runtime_error(ret.error());
            return ret.ok();
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
