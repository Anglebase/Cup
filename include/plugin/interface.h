#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <optional>
#include "cmd/args.h"
namespace fs = std::filesystem;
#ifdef _DEBUG
#include <iostream>
#endif

struct NewData
{
    // The project name parsed from command-line parameters.
    std::string name;
    // The project type parsed from command-line parameters.
    std::string type;
    // The execution root directory parsed from command-line parameters.
    fs::path root;
};

/// @brief Context for generating CMakeLists.txt
/// @note All of the paths are absolute paths.
struct CMakeContext
{
    // The project name parsed from the configuration file.
    std::string name;
    // The required version of CMake for the project.
    // Modify this value through member function `set_cmake_version`.
    std::pair<int, int>& cmake_version;
    // The path where the project is located.
    fs::path current_dir;
    // The path where the constructed project is located.
    // For non dependent items, it has the same value as `current_dir`.
    // For dependencies, it indicates the root project that depends on this package,
    // which is the path where the project being built is located.
    fs::path root_dir;
    // All features that have been resolved and expanded.
    // The plugin can be generated based on the conditions of the CMake.
    std::vector<std::string> features;
    // The dependency key names that actually take effect after version merging and feature control.
    std::set<std::string> dependencies;

    void set_cmake_version(int major, int minor) const
    {
        if (major > cmake_version.first || (major == cmake_version.first && minor > cmake_version.second))
            cmake_version = {major, minor};
    }
};

struct RunProjectData
{
    // The execution target from the command function parameter, which is `cmd` in `cup run <cmd>`.
    // Plugins can decide how to interpret them themselves.
    std::optional<std::string> command;
    // The directory where the project is located.
    fs::path root;
    // The name of the project.
    std::string name;
    // Indicate the current build mode.
    bool is_debug = true;
};

class IPlugin
{
public:
    /// @brief This interface should return the registered name of the plugin.
    /// @return Name of the plugin.
    virtual std::string getName(std::optional<std::string> &except) const = 0;
    /// @brief This interface should implement the generation logic of the template sample project.
    /// @param data Context data.
    /// @return Exit code.
    /// @note This function will be called when cup executes `cup new <name> --type <your plugin>`
    ///       and directly returns the value as program exit code. In the absence of any exceptions,
    ///       it should be zero.
    virtual int run_new(const NewData &data, std::optional<std::string> &except) = 0;
    /// @brief This interface should implement the generation logic of the CMakeLists.txt.
    /// @param context Context data.
    /// @param is_dependency Indicate whether it is used as a dependency when calling the generation of CMake logic.
    /// @return CMakeLists.txt content.
    virtual std::string gen_cmake(const CMakeContext &context, bool is_dependency, std::optional<std::string> &except) = 0;
    /// @brief The return value of this interface will be used as the file content before the project command in the CMake file.
    /// @param context Context data.
    /// @param is_dependency Indicate whether it is used as a dependency when calling the generation of CMake logic.
    /// @return CMakeLists.txt content.
    virtual std::string gen_cmake_global(const CMakeContext &context, bool is_dependency, std::optional<std::string> &except) { return ""; }
    /// @brief This function should return the absolute path of the executable file to be executed.
    /// @param data Context data.
    /// @return Absolute path of a valid executable file.
    /// @note When executing an executable program through the cup run command,
    ///        this function is called and should return the absolute path of a valid executable file.
    virtual fs::path run_project(const RunProjectData &data, std::optional<std::string> &except) = 0;
    /// @brief This function should return the name of the CMake instance.
    /// @param data Context data.
    /// @return Name of the CMake instance.
    /// @note When the build target is specified through the cup run command,
    ///       cup calls this function to obtain the instance name of the build target in CMake,
    ///       which is passed as the --target option parameter of the cmake command-line tool.
    virtual std::optional<std::string> get_target(const RunProjectData &data, std::optional<std::string> &except) const = 0;
    /// @brief Display the help information that comes with the plugin.
    /// @param command The raw parameters parsed from the command line.
    /// @return Exit code.
    /// @note This function is called when the plugin is executed with the `cup help @@<plugin-name>` command.
    ///       The plugin can implement its own help information display logic here.
    virtual int show_help(const cmd::Args &command, std::optional<std::string> &except) const = 0;
    /// @brief Execute the command specified in the command line.
    /// @param command The raw parameters parsed from the command line.
    /// @return Exit code.
    /// @note This function is called when the plugin is executed with the `cup run @@<plugin-name>` command.
    ///       The plugin can implement its own command execution logic here.
    virtual int execute(const cmd::Args &command, std::optional<std::string> &except)
    {
        except = "This plugin does not support command execution.";
        return -1;
    };
};

#define EXPORT_PLUGIN(impl)                                    \
    extern "C"                                                 \
    {                                                          \
        IPlugin *createPlugin() { return new impl(); }         \
        void destroyPlugin(IPlugin *plugin) { delete plugin; } \
    }
