#include "result.h"
#include <iostream>

int main()
{
    auto r1 = Ok<std::string>(42);
    if (r1.is_ok())
        std::cout << "Result is: " << r1.ok() << std::endl;
    else
        std::cout << "Error is: " << r1.error() << std::endl;
    auto r2 = Err<int, std::string>("Error");
    if (r2.is_ok())
        std::cout << "Result is: " << r2.ok() << std::endl;
    else
        std::cout << "Error is: " << r2.error() << std::endl;

    return 0;
}