#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

constexpr std::size_t INSERTION_SORT_THRESHOLD = 16;
constexpr std::size_t TEST_ARRAY_SIZE = 10000;

// Insertion sort for small chunks
template <typename T>
void order(std::vector<T>& data, std::size_t left, std::size_t right)
{
    if (right - left <= 1)
    {
        return;
    }

    for (auto i = left + 1; i < right; ++i)
    {
        for (auto j = i; j > left; --j)
        {
            if (data[j - 1] > data[j])
            {
                std::swap(data[j], data[j - 1]);
            }
            else
            {
                break;
            }
        }
    }
}

// Hoare partition scheme
template <typename T>
std::size_t partition_hoare(std::vector<T>& data, std::size_t left, std::size_t right)
{
    std::size_t middle = std::midpoint(left, right - 1);

    if (data[middle] < data[left])
    {
        std::swap(data[middle], data[left]);
    }
    if (data[right - 1] < data[left])
    {
        std::swap(data[right - 1], data[left]);
    }
    if (data[right - 1] < data[middle])
    {
        std::swap(data[right - 1], data[middle]);
    }

    T pivot = data[middle];
    auto i = left;
    auto j = right - 1;

    while (true)
    {
        while (data[i] < pivot)
        {
            ++i;
        }

        while (data[j] > pivot)
        {
            --j;
        }

        if (i >= j)
        {
            return j;
        }

        std::swap(data[i], data[j]);

        ++i;
        --j;
    }
}

// Hybrid quick sort algorithm
template <typename T>
void quick_sort_hybrid(std::vector<T>& data, std::size_t left, std::size_t right)
{
    if (right - left > INSERTION_SORT_THRESHOLD)
    {
        std::size_t split_point = partition_hoare(data, left, right);
        quick_sort_hybrid(data, left, split_point + 1);
        quick_sort_hybrid(data, split_point + 1, right);
    }
    else
    {
        order(data, left, right);
    }
}

// Main sort interface
template <typename T>
void sort(std::vector<T>& data)
{
    if (data.empty())
    {
        return;
    }
    quick_sort_hybrid(data, 0, data.size());
}

// Google Test fixture class
template <typename T>
class SortTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_size = TEST_ARRAY_SIZE;
        m_data.resize(m_size);
    }

    std::size_t m_size{0};
    std::vector<T> m_data{};
};

// Define types for typed tests
using NumericTypes = ::testing::Types<int, double>;
TYPED_TEST_SUITE(SortTest, NumericTypes);

// Test sorting on a reversed array for multiple numeric types
TYPED_TEST(SortTest, ReversedArray)
{
    for (std::size_t i = 0; i < this->m_size; ++i)
    {
        this->m_data[i] = static_cast<TypeParam>(this->m_size - i);
    }

    sort(this->m_data);

    EXPECT_TRUE(std::ranges::is_sorted(this->m_data));
}

// Test sorting on an array of strings
TEST(StringSortTest, BasicStrings)
{
    std::vector<std::string> words = { "zebra", "apple", "mango", "banana", "orange" };

    sort(words);

    EXPECT_TRUE(std::ranges::is_sorted(words));
}

// Test edge case: empty array
TEST(EdgeCaseTest, EmptyVector)
{
    std::vector<int> empty_data{};

    sort(empty_data);

    EXPECT_TRUE(empty_data.empty());
}

// Entry point for tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}