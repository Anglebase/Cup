#include "cmake.h"
#include <iostream>

int main()
{
    cmake::Generator gen;
    gen.project("cup");
    gen.add_executable("cup", {"src/main.cpp", "src/config.cpp", "src/log.cpp"});

    std::cout << gen.generator() << std::endl;

    cmake::Execute exec;
    exec.build_dir("build");
    exec.generator("Visual Studio 16 2019");

    std::cout << exec.as_command() << std::endl;
    return 0;
}