//g++ -std=c++23 -Wall -Wextra -Wpedantic 11.01.cpp -o 11.01.out

#include <iostream>

class Wrapper;

typedef Wrapper (*TestFunctionPtr)();

class Wrapper {
public:

    Wrapper(TestFunctionPtr func_ptr) : func_ptr_(func_ptr) {}

    operator TestFunctionPtr() const {
        return func_ptr_;
    }

private:
    TestFunctionPtr func_ptr_;
};

Wrapper test() {
    std::cout << "test() called.\n";
    return Wrapper(&test);
}

void run_tests_and_demonstration() {
    std::cout << "\n--- Running Tests and Demonstration ---\n";

    // Test 1: Verifies the function can be called and its message is printed.
    std::cout << "Test 1: Direct call to test()\n";
    test();

    // Test 2: Verifies the self-returning mechanism via the Wrapper class.
    std::cout << "Test 2: Calling test() and then the returned pointer via Wrapper\n";
    Wrapper res = test();
    TestFunctionPtr ptr = res;
    ptr();

    // Test 3: Demonstrates the requested syntax using the overloaded conversion operator.
    std::cout << "Test 3: Wrapper function = test(); (*function)();\n";
    Wrapper function = test();
    (*function)();

    std::cout << "\n--- All tests passed successfully! ---\n";
}

int main() {
    run_tests_and_demonstration();
    return 0;
}