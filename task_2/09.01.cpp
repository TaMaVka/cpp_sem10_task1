//g++ -std=c++20 -Wall -Wextra -Wpedantic 09.01.cpp -o 09.01_debug.out
//g++ -std=c++20 -Wall -Wextra -Wpedantic -DNDEBUG 09.01.cpp -o 09.01_release.out

#include <iostream>
#include <source_location>

// Tracer class implements the RAII pattern to trace function entry and exit.
class Tracer
{
public:
    // Constructor prints the entry message.
    explicit Tracer(const std::source_location& location = std::source_location::current()) :
        m_location(location)
    {
        std::cout << "-> Entering function '" << m_location.function_name()
                  << "' at " << m_location.file_name()
                  << ":" << m_location.line() << std::endl;
    }

    // Destructor prints the exit message.
    ~Tracer()
    {
        std::cout << "<- Leaving function '" << m_location.function_name()
                  << "' from " << m_location.file_name()
                  << ":" << m_location.line() << std::endl;
    }

    // Disable copying and moving to prevent unexpected behavior.
    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;
    Tracer(Tracer&&) = delete;
    Tracer& operator=(Tracer&&) = delete;

private:
    std::source_location m_location;
};

// The 'trace' macro enables or disables tracing based on the NDEBUG symbol.
#ifdef NDEBUG
    #define trace ((void)0)
#else
    #define trace Tracer tracer_object_on_line_##__LINE__
#endif

// --- Demonstration ---

void function_b()
{
    trace;
    std::cout << "   Executing function_b..." << std::endl;
}

void function_a()
{
    trace;
    std::cout << "   Executing function_a, calling function_b..." << std::endl;
    function_b();
    std::cout << "   Returned from function_b." << std::endl;
}

int main()
{
    trace;
    std::cout << "Demonstration of Tracer." << std::endl;
    std::cout << "Calling function_a..." << std::endl;
    function_a();
    std::cout << "Returned from function_a." << std::endl;
    std::cout << "End of main." << std::endl;

    return 0;
}