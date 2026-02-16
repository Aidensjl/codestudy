#include "ModuleA.hpp"
#include "ModuleB.hpp"

#include <iostream>
#include <string>

int main()
{
    const std::string messageA = build_messageA();
    const std::string messageB = build_messageB();

    std::cout << messageA << std::endl;
    std::cout << messageB << std::endl;
    return 0;
}
