#define DEBUG_CONFIG
#include "config.h"
#include <iostream>

int main()
{
    try
    {
        Config config("./target");
        std::cout << config.need<int>("build.c", 17) << std::endl;
        std::cout << config.get<int>("build.cxx") << std::endl;
        std::cout << config.get<std::string>("name") << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}