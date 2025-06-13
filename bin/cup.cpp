#include <iostream>
#include "cmd.h"
#include "log.h"
#include <unordered_map>
#include <functional>
#include "tools.h"
#include "utils.h"

const char *HELP_INFO = R"(Usage:
    cup <command> [args...]

Commands:
    cup new <project name> [options...] 
        Creates a new C++ project. 
        <project name> specifies the project's name.
        options can include:
            --target <target>   Specifies the project type (default: binary).
            --dir <path>        Specifies the project's storage path (default: current directory).

    cup build [build target] [options...] 
        Builds the project. 
        build target specifies the build target. 
        If not specified, builds all targets. 
        This should specify the path of the source file containing the
        main function relative to the bin directory. 
        options can include:
            -r or -release      Builds the project in Release mode.
            --dir <path>        Specifies the project's path.
            --build <path>      Specifies the build directory's path.

    cup clean [options...]
        Cleans the current project's build files. 
        options can include:
            --dir <path>        Specifies the project's path.

    cup run <build target> [options...]
        Runs the project's executable. 
        build target specifies the target to run, corresponding to the
        source file containing the main function relative to the bin directory. 
        options can include:
            -r or -release      Runs the Release build.
            --dir <path>        Specifies the project's path.
            --build <path>      Specifies the build directory's path.
            --args <args...>    Specifies arguments to pass to the executable.

    cup list <option> [options...]
        Lists one of the following properties for the specified Cup project:
            include             All header file directories referenced by the project (absolute paths).
            deps                List of projects this project depends on.
        options can include:
            --dir <path>        Specifies the project's path.

    cup install <url|unique> [options...]
        Installs a Cup package.
        <url> specifies the repository URL to install from.
        <unique> specifies the unique identifier(@<user>/<repo>) of the package on Gitbub to install.
        options can include:
            --version <version> Specifies the version of the package to install.
                                If not specified, installs the latest version.
    
    cup uninstall <url|unique> [options...]
        Uninstalls a Cup package.
        <url> specifies the repository URL to uninstall from.
        <unique> specifies the unique identifier(@<user>/<repo>) of the package on Gitbub to uninstall.
        options can include:
            --version <version> Specifies the version of the package to uninstall.
                                If not specified, uninstalls all versions of the package.
)";

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
    const auto cup_cache = get_user_dir() / ".cup";
    if (!fs::exists(cup_cache))
        fs::create_directories(cup_cache);

    if (args.getArguments().empty())
    {
        LOG_DEBUG("Help| Error: {No Cmd}");
        std::cout << HELP_INFO << std::endl;
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
        {"help", [&]()
         {
             std::cout << HELP_INFO << std::endl;
             return 0;
         }},
        {"install", [&]()
         {
             InstallCmd installCmd(args);
             return installCmd.run();
         }},
        {"uninstall", [&]()
         {
             UninstallCmd uninstallCmd(args);
             return uninstallCmd.run();
         }},
        {"version", [&]()
         {
             std::cout << "Cup version " << _VER_X << "." << _VER_Y << "." << _VER_Z << std::endl;
             return 0;
         }},
    };
    if (table.find(cmd) == table.end())
    {
        LOG_ERROR("Unknown command: ", cmd);
        std::cout << HELP_INFO << std::endl;
        return 1;
    }
    try
    {
        return table[cmd]();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(e.what());
        return 1;
    }
}
