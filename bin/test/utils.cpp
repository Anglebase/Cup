#include <iostream>
#include "utils/utils.h"

int main()
{
    std::string str = "hello,world,how,are,you";
    std::vector<std::string> vec = split(str, ",");
    for (auto s : vec)
    {
        std::cout << s << std::endl;
    }
    std::string str2 = join(vec, ";");
    std::cout << str2 << std::endl;
    return 0;
}