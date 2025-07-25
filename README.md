# Cup: Lightweight & Efficient C++ Project & Package Manager

[![GitHub License](https://img.shields.io/github/license/Anglebase/Cup)](https://github.com/Anglebase/Cup/blob/master/LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/Anglebase/Cup)](https://github.com/Anglebase/Cup/releases)

**Simplify and standardize C++ development.** Inspired by Rust's Cargo, Cup brings modern project management and dependency handling to the rich C++ ecosystem.

## Why Cup?

While C++ boasts a mature ecosystem, project setup and dependency management often involve complex, inconsistent workflows. Cup solves this by abstracting CMake complexities through:

1.  **CMake Abstraction**: Dynamically generates tailored CMake scripts based on standardized project structures
2.  **Minimal Configuration**: Define projects (name/type/dependencies) in simple config files – no CMake expertise needed
3.  **Extensible Plugins**: Adapt to specialized build scenarios (e.g., Qt apps) via lightweight plugins

For most projects, built-in plugins suffice, while the extension system handles advanced use cases.

## Quick Start

### Installation

1.  Download the standalone `cup` binary from [Releases](https://github.com/Anglebase/Cup/releases)  
    **OR** run:  
    ```bash
    cup install @Anglebase/Cup && cup build -r
    ```
2.  Move the binary to your system path (e.g., `~/bin/` or `/usr/local/bin/`)
3.  Ensure the directory is in your `PATH`

### Core Commands

| Command                              | Description                                     |
| :----------------------------------- | :---------------------------------------------- |
| `cup new <name> [--type <type>]`     | Create new project (default: binary)            |
| `cup build`                          | Build current project                           |
| `cup clean`                          | Clean build artifacts                           |
| `cup run`                            | Build & execute the project (if applicable)     |
| `cup install [pkg@url]`              | Install dependency locally                      |
| `cup uninstall [pkg@url]`            | Uninstall local dependency                      |
| `cup list`                           | Show information (e.g., installed packages)     |

### Built-in Project Types (Plugins)

| Type          | Output                          |
| :------------ | :------------------------------ |
| **`binary`**  | Executable (default)            |
| **`static`**  | Static library (.a/.lib)        |
| **`shared`**  | Shared library (.so/.dll)       |
| **`module`**  | Module library (plugins)        |
| **`interface`** | Header-only library             |

## Explore Further

📚 [**Full Documentation**](https://github.com/Anglebase/Cup/tree/master/docs/cup.md)  
Advanced features and customization guides

**Contribute**:  
Submit issues, suggestions, or code via [GitHub Repository](https://github.com/Anglebase/Cup)