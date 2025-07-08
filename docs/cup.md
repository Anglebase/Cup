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

See [default](https://github.com/Anglebase/Cup/blob/master/docs/default.toml)

## Plugins

Cup determines which plugin to call based on the `project. type` field in the project configuration file. The plugin mechanism of Cup allows users to customize builder plugins, and Cup retrieves the `$HOME/.cup/plugins` directory and loads them when used. Cup has five built-in plugins: `binary`, `static`, `shared`, `module` and `interface`, which are used to generate executable programs, static libraries, dynamic libraries, module libraries(plugins) and interface libraries(without source files) respectively.
### API

The plugin must implement the following interfaces:

See [Interface](https://github.com/Anglebase/Cup/blob/master/include/plugin/interface.h)