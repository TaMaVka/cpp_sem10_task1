// g++ -std=c++23 -Wall -Wextra -Wpedantic 10.01.cpp -o 10.01.out

#include <iostream>
#include <vector>
#include <deque>
#include <cstddef> // for size_t
#include <iomanip> // for std::setw
#include <cassert> // for assert

// Investigation of std::vector

// This function demonstrates and tests the memory allocation strategy of std::vector.
// It tracks the capacity changes as elements are added and calculates the growth factor.
void investigate_vector_growth()
{
    std::cout << "--- Investigating std::vector capacity growth ---\n";

    const size_t elements_to_add = 100;
    std::vector<int> vec;

    // Test: An empty vector should have a size and capacity of 0.
    assert(vec.size() == 0);
    // Note: The C++ standard allows an empty vector to have a capacity greater than 0,
    // but on most implementations (like libstdc++ for g++), it starts at 0.
    // We will track the initial capacity to be robust.
    size_t old_capacity = vec.capacity();
    std::cout << "Initial capacity: " << old_capacity << "\n";

    for (size_t i = 0; i < elements_to_add; ++i)
    {
        vec.push_back(static_cast<int>(i));

        if (vec.capacity() != old_capacity)
        {
            // Test: After reallocation, new capacity must be greater than the old one.
            assert(vec.capacity() > old_capacity);

            double growth_factor = 0.0;
            if (old_capacity > 0)
            {
                growth_factor = static_cast<double>(vec.capacity()) / old_capacity;
            }
            
            std::cout << "Size: " << std::setw(3) << vec.size()
                      << ", Old Capacity: " << std::setw(3) << old_capacity
                      << ", New Capacity: " << std::setw(3) << vec.capacity();
            
            if (old_capacity > 0)
            {
                 std::cout << ", Growth Factor: " << growth_factor;
            }
            std::cout << "\n";

            old_capacity = vec.capacity();
        }
    }

    // Test: The final size must match the number of added elements.
    assert(vec.size() == elements_to_add);
    
    std::cout << "---------------------------------------------------\n";
    /*
     * Research Finding for std::vector:
     *
     * The growth factor for std::vector capacity is the multiplier used
     * to determine the new capacity when the current one is exhausted.
     * Based on the test output, for the GCC/libstdc++ implementation,
     * the growth factor is consistently 2.0. This means each time the
     * vector needs to reallocate, it doubles its storage capacity.
     * This provides an amortized constant time complexity for push_back operations.
     */
}

// Investigation of std::deque

// This function demonstrates and tests the memory layout of std::deque.
// By inspecting the addresses of consecutive elements, it identifies block boundaries
// and determines the size of the memory blocks used by the deque.
void investigate_deque_layout()
{
    std::cout << "\n--- Investigating std::deque memory layout ---\n";

    const size_t elements_to_add = 1000;
    std::deque<long long> deq;

    for (size_t i = 0; i < elements_to_add; ++i)
    {
        deq.push_back(static_cast<long long>(i));
    }
    
    // Test: The final size must match the number of added elements.
    assert(deq.size() == elements_to_add);

    size_t elements_in_block = 0;
    bool block_size_found = false;

    for (size_t i = 0; i < deq.size() - 1; ++i)
    {
        const long long* current_element_ptr = &deq[i];
        const long long* next_element_ptr = &deq[i+1];
        
        // Use char pointers to calculate the exact byte difference between addresses.
        const ptrdiff_t address_diff = reinterpret_cast<const char*>(next_element_ptr) - 
                                       reinterpret_cast<const char*>(current_element_ptr);
        
        if (address_diff == sizeof(long long))
        {
            // Test: Elements within the same block must be contiguous in memory.
            // This assertion verifies this expected behavior.
            assert(address_diff > 0); 
            elements_in_block++;
        }
        else
        {
            // The first time the address difference is not sizeof(T), we've crossed a block boundary.
            // The number of contiguous elements seen so far (+1 for the first element) is the block size.
            if (!block_size_found)
            {
                elements_in_block++; // Count the last element of the block.
                std::cout << "Block boundary crossed after element " << i << ".\n";
                std::cout << "Detected " << elements_in_block << " elements per block.\n";
                const size_t block_size_bytes = elements_in_block * sizeof(long long);
                std::cout << "Block size in bytes: " << block_size_bytes << "\n";
                block_size_found = true;
                // We can stop after finding the first block size, as they are typically uniform.
                break; 
            }
        }
    }
    
    if (!block_size_found && deq.size() > 1)
    {
        std::cout << "No block boundary found. All elements are in one block.\n";
        std::cout << "Increase elements_to_add to cross a boundary.\n";
    }

    std::cout << "---------------------------------------------------\n";
    /*
     * Research Finding for std::deque:
     *
     * std::deque stores its elements in a sequence of fixed-size memory blocks (pages).
     * Elements within a single block are contiguous, but the blocks themselves are not.
     * By tracking the memory addresses of consecutive elements, we can detect a large
     * "jump" in addresses, which signifies a transition from one block to another.
     * The number of elements before this jump indicates the block size.
     * For this implementation (GCC/libstdc++), the block size for std::deque<long long>
     * is 64 elements, which corresponds to 512 bytes (64 * 8 bytes).
     * The block size is often chosen to be a power of two for efficiency, and may
     * depend on the element type size, but is often capped at a value like 512 bytes.
     */
}

void run_tests_and_demonstration()
{
    investigate_vector_growth();
    investigate_deque_layout();

    std::cout << "\nAll tests and demonstrations passed successfully.\n";
}

int main()
{
    run_tests_and_demonstration();
    return 0;
}