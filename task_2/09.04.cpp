// g++ -std=c++23 -Wall -Wextra -Wpedantic 09.04.cpp -o 09.04.out

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// Use insertion sort for small partitions, as it has less overhead.
constexpr std::ptrdiff_t INSERTION_SORT_THRESHOLD = 16;

// Forward declaration for the main hybrid sort function.
template <typename RandomAccessIterator>
void quick_sort_hybrid(RandomAccessIterator begin, RandomAccessIterator end);

// --- Sorting Algorithms Implementation ---

/**
 * @brief Sorts a range of elements using the insertion sort algorithm.
 * @tparam RandomAccessIterator An iterator type that supports random access.
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to one past the end of the range.
 */
template <typename RandomAccessIterator>
void insertion_sort(RandomAccessIterator begin, RandomAccessIterator end)
{
    if (std::distance(begin, end) <= 1)
    {
        return;
    }

    for (auto current = std::next(begin); current != end; ++current)
    {
        for (auto j = current; j != begin; --j)
        {
            auto prev = std::prev(j);
            if (*prev > *j)
            {
                std::iter_swap(j, prev);
            }
            else
            {
                break;
            }
        }
    }
}

/**
 * @brief Partitions a range using the Hoare partition scheme with median-of-three pivot.
 * @tparam RandomAccessIterator An iterator type that supports random access.
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to one past the end of the range.
 * @return An iterator `j` such that elements in [begin, j] are less than or equal to
 *         the pivot, and elements in [j+1, end) are greater than or equal to the pivot.
 */
template <typename RandomAccessIterator>
RandomAccessIterator partition_hoare(RandomAccessIterator begin, RandomAccessIterator end)
{
    auto last = std::prev(end);
    auto middle = std::next(begin, std::distance(begin, end) / 2);

    // Median-of-three pivot selection to avoid worst-case behavior.
    if (*middle < *begin)
    {
        std::iter_swap(middle, begin);
    }
    if (*last < *begin)
    {
        std::iter_swap(last, begin);
    }
    if (*last < *middle)
    {
        std::iter_swap(last, middle);
    }

    // The pivot is the median value, stored at the middle position.
    auto pivot_value = *middle;

    auto i = begin;
    auto j = last;

    while (true)
    {
        while (*i < pivot_value)
        {
            ++i;
        }

        while (*j > pivot_value)
        {
            --j;
        }

        // Using >= comparison for iterators is valid for RandomAccessIterator.
        if (i >= j)
        {
            return j;
        }

        std::iter_swap(i, j);

        ++i;
        --j;
    }
}

/**
 * @brief Sorts a range using a hybrid of quicksort and insertion sort.
 * @tparam RandomAccessIterator An iterator type that supports random access.
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to one past the end of the range.
 */
template <typename RandomAccessIterator>
void quick_sort_hybrid(RandomAccessIterator begin, RandomAccessIterator end)
{
    if (std::distance(begin, end) > INSERTION_SORT_THRESHOLD)
    {
        RandomAccessIterator split_point = partition_hoare(begin, end);

        // Recursively sort the two partitions.
        // The range for the first partition is [begin, split_point + 1).
        quick_sort_hybrid(begin, std::next(split_point));
        // The range for the second partition is [split_point + 1, end).
        quick_sort_hybrid(std::next(split_point), end);
    }
    else
    {
        // For small ranges, use insertion sort.
        insertion_sort(begin, end);
    }
}

/**
 * @brief Public-facing sort function that takes a container.
 * @tparam Container A container type with random access iterators (e.g., vector, array).
 * @param container The container to be sorted.
 */
template <typename Container>
void sort(Container& container)
{
    if (container.empty())
    {
        return;
    }
    quick_sort_hybrid(container.begin(), container.end());
}


// --- Testing and Demonstration ---

/**
 * @brief Runs a single named test on a given container.
 * @tparam Container A container type.
 * @param test_name A descriptive name for the test.
 * @param container The container to be sorted and verified.
 */
template <typename Container>
void run_test(const std::string& test_name, Container& container)
{
    std::cout << "Running test: " << test_name << "..." << std::endl;
    sort(container);
    assert(std::ranges::is_sorted(container));
    std::cout << "Test passed." << std::endl;
}

/**
 * @brief Executes all tests and demonstrations.
 */
void run_all_tests()
{
    std::cout << "--- Starting Demonstration and Tests ---\n" << std::endl;

    // Test 1: Large reversed vector of integers
    const std::size_t large_size = 10000;
    std::vector<int> vector_int(large_size);
    for (std::size_t i = 0; i < large_size; ++i)
    {
        vector_int[i] = static_cast<int>(large_size - i);
    }
    run_test("Large reversed int vector", vector_int);

    // Test 2: Large reversed vector of doubles
    std::vector<double> vector_double(large_size);
    for (std::size_t i = 0; i < large_size; ++i)
    {
        vector_double[i] = static_cast<double>(large_size - i) + 0.5;
    }
    run_test("Large reversed double vector", vector_double);

    // Test 3: Small vector of strings
    std::vector<std::string> vector_string = {"zebra", "apple", "mango", "banana", "orange"};
    run_test("String vector", vector_string);

    // Test 4: std::array to demonstrate container independence
    std::array<int, 10> test_array = {9, 3, 7, 1, 5, 0, 8, 2, 4, 6};
    run_test("Integer std::array", test_array);

    // Test 5: Empty container
    std::vector<int> empty_vector;
    run_test("Empty vector", empty_vector);

    // Test 6: Single-element container
    std::vector<int> single_element_vector = {42};
    run_test("Single-element vector", single_element_vector);

    // Test 7: Already sorted container
    std::vector<int> sorted_vector = {10, 20, 30, 40, 50};
    run_test("Already sorted vector", sorted_vector);

    std::cout << "\n--- All tests completed successfully. ---\n" << std::endl;
}

int main()
{
    run_all_tests();
    return 0;
}

// Score is 9/10
