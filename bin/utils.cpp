#include <iostream>
#include "utils.h"

int main()
{
    fs::path path = "e:/Project/cup/bin/utils.cpp";
    auto rep = replace_finally_name(path, "bin", "target");
    std::cout << rep.string() << std::endl;

    fs::path path2 = "e:/Project/cup/bin/bin/utils.cpp";
    auto rep2 = replace_finally_name(path2, "bin", "target");
    std::cout << rep2.string() << std::endl;
    return 0;
}