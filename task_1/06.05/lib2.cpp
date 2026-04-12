#include <iostream>

extern "C" void test()
{
    std::cout << "Test from library 2\n";
}