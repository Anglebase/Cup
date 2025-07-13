# Cup: Streamlined C++ Project & Package Management

[![GitHub License](https://img.shields.io/github/license/Anglebase/Cup)](https://github.com/Anglebase/Cup/blob/master/LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/Anglebase/Cup)](https://github.com/Anglebase/Cup/releases)

**Simplify and standardize your C++ development workflow.** Cup is a powerful tool inspired by Rust's Cargo, designed to bring modern project management and dependency handling to the rich C++ ecosystem.

## Why Cup?

C++ boasts a vast and mature ecosystem, but managing project builds and dependencies can be complex and inconsistent. Cup solves this by providing a consistent, declarative approach:

1.  **CMake Abstraction:** Cup acts as a smart layer over CMake. It dynamically generates tailored CMake scripts based on a standardized project structure and your configuration.
2.  **Simplified Configuration:** Define your project's properties (name, type, dependencies) in a straightforward configuration file. Cup handles the intricate CMake details for you.
3.  **Extensible Plugin System:** Adapt Cup to your specific needs. Build highly customized projects (like Qt applications) or support unique build requirements through easy-to-write plugins. Cup's flexibility grows with your project.

## Get Started

### Installation

1.  Download the standalone `cup` executable from the [Releases](https://github.com/Anglebase/Cup/releases) page OR `cup install @Anglebase/Cup` and `cup build -r`.
2.  Place it in a directory of your choice (e.g., `~/bin/`, `/usr/local/bin/`).
3.  Ensure this directory is included in your system's `PATH` environment variable.

### Core Commands

| Command                               | Description                                                                 |
| :------------------------------------ | :-------------------------------------------------------------------------- |
| `cup new <project-name> [--type <type>]` | Create a new C++ project. Default type is `binary`. See plugin types below. |
| `cup build`                           | Build the current project.                                                  |
| `cup clean`                           | Clean build artifacts from the current project.                             |
| `cup run`                             | Build and run the current project (if applicable).                          |
| `cup install [package]`               | Install a package to the local system.                                      |
| `cup uninstall [package]`             | Uninstall a package from the local system.                                  |
| `cup list`                            | List relevant information (e.g., installed packages).                       |

### Built-in Project Types (Plugins)

Cup comes ready with plugins for common project structures:

*   **`binary`**: Builds executable applications (default).
*   **`static`**: Builds a static library (`.a`, `.lib`).
*   **`shared`**: Builds a shared/dynamic library (`.so`, `.dll`).
*   **`module`**: Builds a module library (plugin).
*   **`interface`**: Builds a non-source files (interface) library.

## Dive Deeper

Explore the comprehensive Cup documentation to master its features:
📚 [**Full Cup Documentation**](https://github.com/Anglebase/Cup/tree/master/docs/cup.md)

**Contribute & Discuss:** Visit the [Anglebase/Cup GitHub repository](https://github.com/Anglebase/Cup) to report issues, suggest features, or contribute code.