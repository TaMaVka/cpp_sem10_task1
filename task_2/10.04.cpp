// g++ -std=c++20 -Wall -Wextra -Wpedantic 10.04.cpp -o 10.04.out

#include <iostream>
#include <cassert>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>

// Type alias for the Fibonacci numbers, as required by the task.

using FiboType = unsigned long long int;

// Type alias for the 2x2 matrix used in calculations.

using FiboMatrix = boost::numeric::ublas::matrix<FiboType>;

/**
 * @brief Computes the power of a matrix using exponentiation by squaring.
 * @param base The matrix to be raised to a power.
 * @param exp The exponent.
 * @return The resulting matrix (base^exp).
 */

FiboMatrix matrix_power(FiboMatrix base, unsigned int exp) {

    // The size of the matrix is fixed at 2x2.
    const unsigned int matrix_size = 2;

    // Start with the identity matrix, which is the multiplicative identity.
    FiboMatrix result = boost::numeric::ublas::identity_matrix<FiboType>(matrix_size);

    while (exp > 0) {

        // If the exponent is odd, multiply the result by the current base.
        if (exp % 2 == 1) {
            result = boost::numeric::ublas::prod(result, base);
        }

        // Square the base matrix.

        base = boost::numeric::ublas::prod(base, base);

        // Halve the exponent.
        exp /= 2;
    }
    return result;
}

/**
 * @brief Calculates the N-th Fibonacci number using matrix exponentiation.
 * @param n The index of the Fibonacci number to compute (F_n).
 * @return The N-th Fibonacci number.
 */

FiboType fibonacci_matrix(unsigned int n) {
    if (n == 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }

    // The fundamental transformation matrix for Fibonacci numbers.
    
    const unsigned int matrix_size = 2;
    FiboMatrix m(matrix_size, matrix_size);
    m(0, 0) = 1; m(0, 1) = 1;
    m(1, 0) = 1; m(1, 1) = 0;

    // We need to calculate M^(n-1) to get F(n) in the top-left corner.
    FiboMatrix result_matrix = matrix_power(m, n - 1);

    return result_matrix(0, 0);
}

void run_tests_and_demonstration() {
    std::cout << "Running tests..." << std::endl;

    // Test: Base cases (F_0 and F_1)
    std::cout << "Test 1: Base cases..." << std::endl;
    assert(fibonacci_matrix(0) == 0);
    assert(fibonacci_matrix(1) == 1);
    std::cout << "Test 1 Passed." << std::endl;

    // Test: Small, well-known Fibonacci numbers
    std::cout << "Test 2: Small numbers..." << std::endl;
    assert(fibonacci_matrix(2) == 1);
    assert(fibonacci_matrix(3) == 2);
    assert(fibonacci_matrix(5) == 5);
    assert(fibonacci_matrix(10) == 55);
    std::cout << "Test 2 Passed." << std::endl;

    // Test: A moderately large number to check performance and correctness
    std::cout << "Test 3: A moderately large number..." << std::endl;
    assert(fibonacci_matrix(40) == 102334155);
    std::cout << "Test 3 Passed." << std::endl;
    
    // Test: The largest Fibonacci number that fits in unsigned long long int (F_93)
    std::cout << "Test 4: Maximum value for unsigned long long int..." << std::endl;
    const FiboType fibo_93 = 12200160415121876738ULL;
    assert(fibonacci_matrix(93) == fibo_93);
    std::cout << "Test 4 Passed." << std::endl;

    std::cout << "\nAll tests passed successfully.\n" << std::endl;

    // Demonstration: Print the first 20 Fibonacci numbers
    std::cout << "Demonstration: First 20 Fibonacci numbers:" << std::endl;
    const int demonstration_count = 20;
    for (int i = 0; i <= demonstration_count; ++i) {
        std::cout << "F(" << i << ") = " << fibonacci_matrix(i) << std::endl;
    }
}

int main() {
    run_tests_and_demonstration();
    return 0;
}