# Cup2

**A powerful C++ project management and package management tool**

[![GitHub License](https://img.shields.io/github/license/Anglebase/Cup)](https://github.com/Anglebase/Cup/blob/master/LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/Anglebase/Cup)](https://github.com/Anglebase/Cup/releases)

## Introduction

The design inspiration for Cup comes from Rust's Cargo tool, but C++ is not a newly born language. It already has a huge ecosystem, and C++ itself does not have a fixed programming paradigm. The introduction of third-party libraries is also chaotic. Cup was born to solve this problem.

The essence of Cup is to template and dynamically generate corresponding CMake script code based on a fixed project structure, and then build it by calling CMake. For users, they only need to modify the information in the configuration file to define the relevant configurations, and all the CMake processes are completely handled by Cup and its plugin system, which greatly simplifies the construction process.

In order to meet the diverse needs of C++ project construction, Cup provides an easily extensible plugin system, which enables the construction of highly customized C++ projects such as Qt through plugin extensions.

## Usage

### Install

The Cup itself only has one executable file, which can be downloaded directly from the distribution list in the repository to your computer, stored in your favorite location, and then added to the environment variables of your computer.

### Commands

+ **`cup new <project-name> [--type <project-type>]`**: Create a new C++ project with the specified name and type. The `type` is the name of plugin, and by default it is `binary`. Cup has 5 built-in plugins:
    + `binary`: A binary(executable file) project.
    + `static`: A static library project.
    + `shared`: A shared library project.
    + `module`: A module library project.
    + `interface`: An interface library project.
+ **`cup build`**: Build the current project.
+ **`cup clean`**: Clean the current project.
+ **`cup install`**: Install a package to the local system.
+ **`cup run`**: Run the current project.
+ **`cup uninstall`**: Uninstall a package from the local system.
+ **`cup list`**: List information.

## Documentation

The complete documentation of Cup can be found in the [docs](https://github.com/Anglebase/Cup/tree/master/docs/cup.md).