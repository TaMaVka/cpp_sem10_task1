#include <iostream>
#include <string>

#include <boost/dll.hpp>

int main()
{
    std::cout << "Enter library name (or 'exit' to quit):\n";

    std::string libname;
    while (std::cin >> libname)
    {
        if (libname == "exit")
        {
            break;
        }

        try
        {   auto func = boost::dll::import_symbol<void()>(libname, "test");
            func();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << '\n';
        }

        std::cout << "\nEnter another library name (or 'exit'):\n";
    }
}