#define DEBUG_CONFIG
#include "config.h"
#include <iostream>

int main()
{
    try
    {
        Config config("./target");
        std::cout << *config.config << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}