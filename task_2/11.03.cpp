//g++ -std=c++23 -Wall -Wextra -Wpedantic 11.03.cpp -o 11.03.out

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

constexpr std::ptrdiff_t INSERTION_SORT_THRESHOLD = 16;

template <typename RandomAccessIterator, typename Comparator>
void quick_sort_hybrid(RandomAccessIterator begin, RandomAccessIterator end, Comparator comp);

// --- Sorting Algorithms Implementation ---

/**
 * @brief Sorts a range of elements using the insertion sort algorithm.
 * @tparam RandomAccessIterator An iterator type that supports random access.
 * @tparam Comparator A callable type that defines the comparison logic.
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to one past the end of the range.
 * @param comp The comparator object used for element comparison.
 */

template <typename RandomAccessIterator, typename Comparator>
void insertion_sort(RandomAccessIterator begin, RandomAccessIterator end, Comparator comp)
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
            if (comp(*j, *prev)) // Use comparator here
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
 * @tparam Comparator A callable type that defines the comparison logic.
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to one past the end of the range.
 * @param comp The comparator object used for element comparison.
 * @return An iterator `j` such that elements in [begin, j] are less than or equal to
 *         the pivot, and elements in [j+1, end) are greater than or equal to the pivot.
 */

template <typename RandomAccessIterator, typename Comparator>
RandomAccessIterator partition_hoare(RandomAccessIterator begin, RandomAccessIterator end, Comparator comp)
{
    auto last = std::prev(end);
    auto middle = std::next(begin, std::distance(begin, end) / 2);

    if (comp(*middle, *begin))
    {
        std::iter_swap(middle, begin);
    }
    if (comp(*last, *begin))
    {
        std::iter_swap(last, begin);
    }
    if (comp(*last, *middle))
    {
        std::iter_swap(last, middle);
    }

    // The pivot is the median value, stored at the middle position.
    auto pivot_value = *middle;

    auto i = begin;
    auto j = last;

    while (true)
    {
        while (comp(*i, pivot_value))
        {
            ++i;
        }

        while (comp(pivot_value, *j))
        {
            --j;
        }

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
 * @tparam Comparator A callable type that defines the comparison logic.
 * @param begin Iterator to the beginning of the range.
 * @param end Iterator to one past the end of the range.
 * @param comp The comparator object used for element comparison.
 */

template <typename RandomAccessIterator, typename Comparator>
void quick_sort_hybrid(RandomAccessIterator begin, RandomAccessIterator end, Comparator comp)
{
    if (std::distance(begin, end) > INSERTION_SORT_THRESHOLD)
    {
        RandomAccessIterator split_point = partition_hoare(begin, end, comp);

        // Recursively sort the two partitions.
        // The range for the first partition is [begin, split_point + 1).
        quick_sort_hybrid(begin, std::next(split_point), comp);

        // The range for the second partition is [split_point + 1, end).
        quick_sort_hybrid(std::next(split_point), end, comp);
    }
    else
    {
        // For small ranges, use insertion sort.
        insertion_sort(begin, end, comp);
    }
}

/**
 * @brief Public-facing sort function that takes a container and a comparator.
 * @tparam Container A container type with random access iterators (e.g., vector, array).
 * @tparam Comparator A callable type that defines the comparison logic.
 * @param container The container to be sorted.
 * @param comp The comparator object used for element comparison.
 */

template <typename Container, typename Comparator>
void sort(Container& container, Comparator comp)
{
    if (container.empty())
    {
        return;
    }
    quick_sort_hybrid(container.begin(), container.end(), comp);
}

// Custom Comparators for Demonstration ---

template <typename T>
bool greater_than(const T& a, const T& b)
{
    return a > b;
}

// --- Testing and Demonstration ---

/**
 * @brief Runs a single named test on a given container with a specified comparator.
 * @tparam Container A container type.
 * @tparam Comparator A callable type that defines the comparison logic.
 * @param test_name A descriptive name for the test.
 * @param container The container to be sorted and verified.
 * @param comp The comparator object used for element comparison.
 */

template <typename Container, typename Comparator>
void run_test(const std::string& test_name, Container& container, Comparator comp)
{
    std::cout << "Running test: " << test_name << "...\n";
    sort(container, comp);
    // Verify sorted order based on the comparator
    // std::ranges::is_sorted can take a custom comparator.
    assert(std::ranges::is_sorted(container, comp));
    std::cout << "Test passed.\n";
}


// Executes all tests and demonstrations.

void run_all_tests()
{
    std::cout << "--- Starting Demonstration and Tests ---\n\n";

    // Test 1: Ascending order with std::less<int>
    std::vector<int> vec1 = {9, 3, 7, 1, 5, 0, 8, 2, 4, 6};
    run_test("Ascending int vector with std::less", vec1, std::less<int>());

    // Test 2: Descending order with a free function comparator
    std::vector<int> vec2 = {9, 3, 7, 1, 5, 0, 8, 2, 4, 6};
    run_test("Descending int vector with free function", vec2, greater_than<int>);

    // Test 3: Ascending order for strings with a lambda comparator
    std::vector<std::string> vec3 = {"zebra", "apple", "mango", "banana", "orange"};
    auto string_comp_lambda = [](const std::string& a, const std::string& b) { return a < b; };
    run_test("Ascending string vector with lambda", vec3, string_comp_lambda);

    // Test 4: Descending order for doubles with a lambda comparator
    std::vector<double> vec4 = {3.14, 1.618, 2.718, 0.577, 1.414};
    auto double_comp_lambda = [](const double& a, const double& b) { return a > b; };
    run_test("Descending double vector with lambda", vec4, double_comp_lambda);

    // Test 5: Empty container with std::less
    std::vector<int> empty_vec;
    run_test("Empty vector with std::less", empty_vec, std::less<int>());

    // Test 6: Single-element container with std::less
    std::vector<int> single_vec = {42};
    run_test("Single-element vector with std::less", single_vec, std::less<int>());

    std::cout << "\n--- All tests completed successfully. ---\n\n";
}


// Main entry point of the application.

int main()
{
    run_all_tests();
    return 0;
}
