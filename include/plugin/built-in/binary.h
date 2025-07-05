#include "plugin/interface.h"
#include "template.h"

/*
Binary Project Builder Plugin

Binary Project is a executable program project which has mutliple executables.

It's directory structure is like:
- ProjectRoot
    - src
        - bin
        + main.cpp
    - include
    - test
    - cup.toml

The `src` directory contains the source files of the project, 
and the `bin` directory contains the main files of the project which contains a `main` function.
The `main.cpp` also contains the `main` function, but it's suffix is not must be `.cpp`.
It can also be `.c` or other valid suffix.

The `include` directory contains the header files of the project.

The `test` directory contains the test files of the project.
All files in the `test` directory will be considered as main file of the project.

The `cup.toml` file is the configuration of the project.

For `src/bin` and `test` directory, it does not recursively search for the main files within it.

Command behavior:
The parameters accepted by the `run` command will be interpreted as the executable program to be executed:
- If not specified or specified as main, specify the default executable program for the project, 
  which is the program compiled from src/main.cpp.
- If bin/... is specified, it will be interpreted as an executable program compiled from the source
  files in src/bin.
- If test/... is specified, it will be interpreted as an executable program compiled from the source
  files in test/.
- Otherwise, it will be considered an error.
*/

class BinaryPlugin : public IPlugin
{
    void _get_source_files(const fs::path &dir, std::vector<fs::path> &files);

public:
    std::string getName() const override;
    int run_new(const NewData &data) override;
    std::string gen_cmake(const CMakeContext &ctx, bool is_dependency) override;
    fs::path run_project(const RunProjectData &data) override;
    std::optional<std::string> get_target(const RunProjectData &data) const override;
};