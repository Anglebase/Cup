# Cup Documentation

## Commands

The command line parameter format for Cup is`cup <subcommand> [args...]`, where`<subcommand>`is a subcommant and, `[args...]`is the optional parameters.
Overview of sub commands:
+ `new`: Create a project.
+ `build`: Build the project.
+ `run`: Run the project.
+ `clean`: Clean up the construction intermediate files of the project.
+ `install`: Install the package.
+ `uninstall`: Uninstall the package.
+ `list`: List the specified information.
+ `help`: Display help information.

### `new`
The command format for this sub command is:
+   `cup new <project-name> [--type <project-type>] [--dir <project-dir>]`

Among them:
+ `project-name`Indicate the project name, which can only consist of uppercase and lowercase letters, numbers, and underscores.
+ `project-type`Indicate the project type, which is used to indicate which builder plugin should be called when the project executes other commands. By default, it is`binary`.
+ `project-dir`Indicate the directory where the project is generated, which by default is the current command execution directory.

### `build`
The command format for this sub command is:
+   `cup build [-r|--release] [--dir <project-dir>]`

Among them:
+ `-r|--release`Indicate the type of build, if this parameter is specified, the type of build is`release`. Otherwise, it is`debug`
+ `project-dir`Indicate the directory where the project is located, which by default is the current command execution directory.

### `run`
The command format for this sub command is:
+   `cup run [target] [-r|--release] [--dir <project-dir>]`

Among them:
+ `target`Indicate the target to be run, this option can be empty. The implementation of the builder plugin determines how this option is interpreted.
+ `-r|--release`Indicate the type of build, if this parameter is specified, the type of build is`release`. Otherwise, it is`debug`
+ `project-dir`Indicate the directory where the project is located, which by default is the current command execution directory.

### `clean`
The command format for this sub command is:
+   `cup clean [--dir <project-dir>]`

Among them:
+ `project-dir`Indicate the directory where the project is located, which by default is the current command execution directory.

### `install`
The command format for this sub command is:
+   `cup install <package-url> [--version <package-version>]`

Among them:
+ `package-url`Indicate the URL of the package to be installed locally, allowing the use of shorthand `@<user>/<repo>` for Github projects.
+ `package-version`Indicate the version of the package to be installed, and if not specified, automatically obtain the latest version for installation.

### `uninstall`
The command format for this sub command is:
+   `cup uninstall <package-url> --version <package-version>`

Among them:
+ `package-url`Indicate the URL of the package to be uninstalled, allowing the use of shorthand `@<user>/<repo>` for Github projects.
+ `package-version`Indicate the version of the package to be uninstalled.

### `list`
The command format for this sub command is:
+   `cup list <info-type>`

The value of `info type` can be:
+ `plugins`Indicate to list all installed plugins.
+ `packages`Indicate to list all installed packages.

## `help`
The command format for this sub command is:
+   `cup help <subcommand>`

Among them, `subcommand` indicate the subcommands to display help information. If this parameter is omitted, an overview of the help information for all subcommands will be displayed.

## Configuration

The interpretation of other content in the configuration file(`cup.toml`) of the Cup project, except for the default content, is defined by the implementation of the builder plugin.

### Default
`cup.toml`'s default configuration options:
```toml
[project]
name = "Project Name" # Required
# Project name.
# It can only be composed of uppercase and lowercase letters, numbers, and underscores.
version = "0.1.0" # Required
# Project version.
# Used to indicate the version number of the project.
type = "binary" # Required
# Project type.
# Used to indicate which builder plugin should be called when the project executes other commands.
# It is the same as the --type parameter specified by the `new` sub command.
license = "MIT"
# Project license.
# Used to indicate license information for the project.

[build]
generator = "MinGW Makefiles"
# Specify the generator used by CMake
# This option is used to control the toolchain called by CMake
# By default, Visual Studio 17 2022 (Windows) Unix Makefiles (Linux) are used
# Please refer to the CMake documentation for other options
defines = ["DEBUG", "ONE=1"]
# Add global macro
# This option adds macro definitions for building projects
includes = ["include"]
# Add additional header file search path
link_dirs = ["link_dir"]
# Add additional link library search paths
link_libs = ["lib1", "lib2"]
# Add additional link libraries
compile_options = ["-Wall", "-Wextra"]
# Add compilation options
link_options = ["-static"]
# Add link options

[build.export]
compile_commands = "compile_commands.json"
# Specify the path to generate the compile_commands.json file
# If empty, it will be located at the default build location (<project-dir>/target/build/cake)
# If this option is not specified, the file will not be generated
# This option is only valid for some CMake generators, please refer to the CMake documentation for details

[dependencies]
name = { path = "path/to/package", version = "1.0.0", url="https://github.com/user/repo", features = ["feature1", "feature2"] }
# Add dependencies
# 
Among them:
# The `name` indicates the name of the dependency item, which is interpreted by Cup as the name referenced in the `link_ibraries` of the build target in CMake
# `path` indicates that the dependency item is a local project path
# `version` indicates the version of the dependency item
# `url` indicates the URL of the dependency item, used for downloading from a remote repository. For Github projects, shorthand `@<user>/<repo>` is allowed`
# `features` indicates the functional feature macro of a dependency item. How to explain that the functional feature macro is defined by the dependency's builder plugin
# At least one of the configuration parameters `path` and `url` needs to be specified
```

## Plugins

Cup determines which plugin to call based on the `project. type` field in the project configuration file. The plugin mechanism of Cup allows users to customize builder plugins, and Cup retrieves the `$HOME/.cup/plugins` directory and loads them when used. Cup has five built-in plugins: `binary`, `static`, `shared`, `module` and `interface`, which are used to generate executable programs, static libraries, dynamic libraries, module libraries(plugins) and interface libraries(without source files) respectively.
### API

The plugin must implement the following interfaces:

```cpp

class IPlugin
{
public:
    /// @brief This interface should return the registered name of the plugin.
    /// @return Name of the plugin.
    virtual std::string getName() const = 0;
    /// @brief This interface should implement the generation logic of the template sample project.
    /// @param data Context data.
    /// @return Exit code.
    /// @note This function will be called when cup executes `cup new <name> --type <your plugin>` 
    ///       and directly returns the value as program exit code. In the absence of any exceptions,
    ///       it should be zero.
    virtual int run_new(const NewData &data) = 0;
    /// @brief This interface should implement the generation logic of the CMakeLists.txt.
    /// @param context Context data.
    /// @param is_dependency Indicate whether it is used as a dependency when calling the generation of CMake logic.
    /// @return CMakeLists.txt content.
    virtual std::string gen_cmake(const CMakeContext &context, bool is_dependency) = 0;
    /// @brief This function should return the absolute path of the executable file to be executed.
    /// @param data Context data.
    /// @return Absolute path of a valid executable file.
    /// @note When executing an executable program through the cup run command, 
    ///        this function is called and should return the absolute path of a valid executable file.
    virtual fs::path run_project(const RunProjectData &data) = 0;
    /// @brief This function should return the name of the CMake instance.
    /// @param data Context data.
    /// @return Name of the CMake instance.
    /// @note When the build target is specified through the cup run command, 
    ///       cup calls this function to obtain the instance name of the build target in CMake, 
    ///       which is passed as the --target option parameter of the cmake command-line tool. 
    virtual std::optional<std::string> get_target(const RunProjectData &data) const = 0;
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

// External exposed interfaces
extern "C" {
    IPlugin* createPlugin();
    void destroyPlugin(IPlugin* plugin);
}
```