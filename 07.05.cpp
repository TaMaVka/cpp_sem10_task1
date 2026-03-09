#include <benchmark/benchmark.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

constexpr std::size_t TEST_ARRAY_SIZE = 10000;
constexpr int64_t MIN_THRESHOLD = 0;
constexpr int64_t MAX_THRESHOLD = 64;
constexpr int64_t THRESHOLD_STEP = 8;

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

// Hybrid quick sort algorithm with threshold parameter
template <typename T>
void quick_sort_hybrid(std::vector<T>& data, std::size_t left, std::size_t right, std::size_t threshold)
{
    // Explicit base case to prevent infinite recursion
    if (right - left <= 1)
    {
        return;
    }

    if (right - left > threshold)
    {
        std::size_t split_point = partition_hoare(data, left, right);
        quick_sort_hybrid(data, left, split_point + 1, threshold);
        quick_sort_hybrid(data, split_point + 1, right, threshold);
    }
    else
    {
        order(data, left, right);
    }
}

// Main sort interface
template <typename T>
void sort(std::vector<T>& data, std::size_t threshold)
{
    if (data.empty())
    {
        return;
    }
    quick_sort_hybrid(data, 0, data.size(), threshold);
}

// Google Benchmark fixture class
class SortBenchmark : public benchmark::Fixture
{
public:
    void SetUp(const benchmark::State& /*state*/) override
    {
        m_size = TEST_ARRAY_SIZE;
        m_base_data.resize(m_size);

        for (std::size_t i = 0; i < m_size; ++i)
        {
            m_base_data[i] = static_cast<double>(m_size - i);
        }
    }

protected:
    std::size_t m_size{0};
    std::vector<double> m_base_data{};
};

// Parameterized microbenchmark
BENCHMARK_DEFINE_F(SortBenchmark, ReverseDoubleSort)(benchmark::State& state)
{
    std::size_t threshold = static_cast<std::size_t>(state.range(0));

    for (auto _ : state)
    {
        state.PauseTiming();
        std::vector<double> data = m_base_data;
        state.ResumeTiming();

        sort(data, threshold);

        benchmark::DoNotOptimize(data);
    }
}

// Register with different thresholds
BENCHMARK_REGISTER_F(SortBenchmark, ReverseDoubleSort)->DenseRange(MIN_THRESHOLD, MAX_THRESHOLD, THRESHOLD_STEP);

// Entry point for benchmark
BENCHMARK_MAIN();