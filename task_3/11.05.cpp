// g++ -std=c++23 -Wall -Wextra -Wpedantic 11.06.cpp -o 11.06.out

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

// --- transform_if: filter by predicate, then transform matching elements ---

template <std::ranges::input_range R,
          std::weakly_incrementable O,
          typename Pred,
          typename Func>
O transform_if(R&& range, O out, Pred pred, Func func) {
    using ValueType = std::ranges::range_value_t<std::remove_cvref_t<R>>;
    std::vector<ValueType> filtered;
    std::ranges::copy_if(std::forward<R>(range),
                         std::back_inserter(filtered), pred);
    return std::ranges::transform(filtered, out, func).out;
}

// --- MAE and MSE via std::transform_reduce ---

double compute_mae(const std::vector<double>& actual,
                   const std::vector<double>& predicted) {
    auto count = static_cast<double>(actual.size());
    return std::transform_reduce(
               actual.begin(), actual.end(), predicted.begin(), 0.0,
               std::plus<>{},
               [](double a, double p) { return std::abs(a - p); }) /
           count;
}

double compute_mse(const std::vector<double>& actual,
                   const std::vector<double>& predicted) {
    auto count = static_cast<double>(actual.size());
    return std::transform_reduce(
               actual.begin(), actual.end(), predicted.begin(), 0.0,
               std::plus<>{},
               [](double a, double p) { return (a - p) * (a - p); }) /
           count;
}

// --- Fibonacci view (CRTP via view_interface) ---

class Fibonacci
    : public std::ranges::view_interface<Fibonacci> {

    class Iterator {
    public:
        using value_type = long long;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        Iterator() : prev_(0), curr_(1), remaining_(0) {}

        explicit Iterator(std::size_t remaining)
            : prev_(0), curr_(1), remaining_(remaining) {}

        const value_type& operator*() const { return prev_; }

        Iterator& operator++() {
            value_type next = prev_ + curr_;
            prev_ = curr_;
            curr_ = next;
            --remaining_;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const {
            return remaining_ == other.remaining_;
        }

        bool operator==(std::default_sentinel_t) const {
            return remaining_ == 0;
        }

    private:
        value_type prev_;
        value_type curr_;
        std::size_t remaining_;
    };

    static_assert(std::forward_iterator<Iterator>);

    std::size_t count_;

public:
    Fibonacci() : count_(0) {}
    explicit Fibonacci(std::size_t count) : count_(count) {}

    Iterator begin() const { return Iterator(count_); }
    std::default_sentinel_t end() const { return {}; }
};

static_assert(std::ranges::view<Fibonacci>);
static_assert(std::ranges::forward_range<Fibonacci>);

// --- Tests for ranges algorithms ---

void test_ranges_replace() {
    // Replace all occurrences of 3 with 0
    std::vector<int> v = {1, 2, 3, 4, 3, 5};
    std::ranges::replace(v, 3, 0);
    assert((v == std::vector<int>{1, 2, 0, 4, 0, 5}));
}

void test_ranges_fill() {
    // Fill entire vector with constant value
    std::vector<int> v(5);
    std::ranges::fill(v, 7);
    assert((v == std::vector<int>{7, 7, 7, 7, 7}));
}

void test_ranges_unique() {
    // Collapse consecutive duplicates
    std::vector<int> v = {1, 1, 2, 2, 2, 3, 1, 1};
    auto tail = std::ranges::unique(v);
    v.erase(tail.begin(), tail.end());
    assert((v == std::vector<int>{1, 2, 3, 1}));
}

void test_ranges_rotate() {
    // Rotate so third element becomes first
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::ranges::rotate(v, v.begin() + 2);
    assert((v == std::vector<int>{3, 4, 5, 1, 2}));
}

void test_ranges_sample() {
    // Sample 3 elements; verify count and membership
    const std::vector<int> src = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> dst;
    constexpr std::size_t sample_count = 3;
    std::mt19937 rng(42);
    std::ranges::sample(src, std::back_inserter(dst), sample_count, rng);
    assert(dst.size() == sample_count);
    for (int x : dst) {
        assert(std::ranges::find(src, x) != src.end());
    }
}

// --- Tests for transform_if ---

void test_transform_if() {
    // Double only even numbers
    const std::vector<int> src = {1, 2, 3, 4, 5, 6};
    std::vector<int> dst;
    auto is_even = [](int x) { return x % 2 == 0; };
    auto doubled = [](int x) { return x * 2; };
    transform_if(src, std::back_inserter(dst), is_even, doubled);
    assert((dst == std::vector<int>{4, 8, 12}));

    // Empty input produces empty output
    const std::vector<int> empty_src;
    std::vector<int> empty_dst;
    transform_if(empty_src, std::back_inserter(empty_dst), is_even, doubled);
    assert(empty_dst.empty());
}

// --- Tests for MAE and MSE ---

void test_mae() {
    // Identical sequences yield zero error
    const std::vector<double> a = {1.0, 2.0, 3.0};
    const std::vector<double> b = {1.0, 2.0, 3.0};
    assert(compute_mae(a, b) == 0.0);

    // Uniform offset of 1 yields MAE = 1
    const std::vector<double> c = {1.0, 2.0, 3.0};
    const std::vector<double> d = {2.0, 3.0, 4.0};
    assert(std::abs(compute_mae(c, d) - 1.0) < 1e-9);
}

void test_mse() {
    // Identical sequences yield zero error
    const std::vector<double> a = {1.0, 2.0, 3.0};
    const std::vector<double> b = {1.0, 2.0, 3.0};
    assert(compute_mse(a, b) == 0.0);

    // Errors of 1, 2, 3 yield MSE = (1+4+9)/3
    const std::vector<double> c = {1.0, 2.0, 3.0};
    const std::vector<double> d = {2.0, 4.0, 6.0};
    constexpr double expected = 14.0 / 3.0;
    assert(std::abs(compute_mse(c, d) - expected) < 1e-9);
}

// --- Tests for views ---

void test_views_filter() {
    // Keep only even numbers
    const std::vector<int> v = {1, 2, 3, 4, 5, 6};
    std::vector<int> result;
    for (int x : v | std::views::filter([](int n) { return n % 2 == 0; })) {
        result.push_back(x);
    }
    assert((result == std::vector<int>{2, 4, 6}));
}

void test_views_drop() {
    // Skip first 3 elements
    const std::vector<int> v = {10, 20, 30, 40, 50};
    std::vector<int> result;
    for (int x : v | std::views::drop(3)) {
        result.push_back(x);
    }
    assert((result == std::vector<int>{40, 50}));
}

void test_views_join() {
    // Flatten nested vectors into a single sequence
    const std::vector<std::vector<int>> nested = {{1, 2}, {3}, {4, 5, 6}};
    std::vector<int> result;
    for (int x : nested | std::views::join) {
        result.push_back(x);
    }
    assert((result == std::vector<int>{1, 2, 3, 4, 5, 6}));
}

void test_views_zip() {
    // Pair elements from two ranges
    const std::vector<int> a = {1, 2, 3};
    const std::vector<char> b = {'a', 'b', 'c'};
    auto zipped = std::views::zip(a, b);
    auto it = zipped.begin();
    assert(std::get<0>(*it) == 1 && std::get<1>(*it) == 'a');
    ++it;
    assert(std::get<0>(*it) == 2 && std::get<1>(*it) == 'b');
    ++it;
    assert(std::get<0>(*it) == 3 && std::get<1>(*it) == 'c');
}

void test_views_stride() {
    // Take every second element
    const std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> result;
    for (int x : v | std::views::stride(2)) {
        result.push_back(x);
    }
    assert((result == std::vector<int>{1, 3, 5, 7}));
}

// --- Tests for Fibonacci view ---

void test_fibonacci_view() {
    constexpr long long expected[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    constexpr std::size_t count = std::size(expected);

    // Verify first 10 Fibonacci numbers
    Fibonacci fib(count);
    std::size_t i = 0;
    for (auto val : fib) {
        assert(val == expected[i]);
        ++i;
    }
    assert(i == count);

    // Empty view reports empty via view_interface
    Fibonacci empty_fib(0);
    assert(empty_fib.empty());

    // Non-empty view provides front() via view_interface
    assert(!fib.empty());
    assert(fib.front() == 0);
}

void test_fibonacci_with_views() {
    constexpr std::size_t total = 10;

    // Drop first 5 Fibonacci numbers
    std::vector<long long> tail;
    for (auto x : Fibonacci(total) | std::views::drop(5)) {
        tail.push_back(x);
    }
    assert((tail == std::vector<long long>{5, 8, 13, 21, 34}));

    // Filter even Fibonacci numbers from first 10
    std::vector<long long> evens;
    auto is_even = [](long long n) { return n % 2 == 0; };
    for (auto x : Fibonacci(total) | std::views::filter(is_even)) {
        evens.push_back(x);
    }
    assert((evens == std::vector<long long>{0, 2, 8, 34}));

    // Stride: every 3rd Fibonacci number from first 10
    std::vector<long long> strided;
    for (auto x : Fibonacci(total) | std::views::stride(3)) {
        strided.push_back(x);
    }
    assert((strided == std::vector<long long>{0, 2, 8, 34}));
}

// --- Entry point ---

void run_tests_and_demonstration() {
    test_ranges_replace();
    test_ranges_fill();
    test_ranges_unique();
    test_ranges_rotate();
    test_ranges_sample();
    test_transform_if();
    test_mae();
    test_mse();
    test_views_filter();
    test_views_drop();
    test_views_join();
    test_views_zip();
    test_views_stride();
    test_fibonacci_view();
    test_fibonacci_with_views();

    std::cout << "All tests passed.\n";

    constexpr std::size_t demo_count = 15;
    std::cout << "Fibonacci(" << demo_count << "): ";
    const char* separator = "";
    for (auto val : Fibonacci(demo_count)) {
        std::cout << separator << val;
        separator = ", ";
    }
    std::cout << "\n";

    std::cout << "Even Fibonacci(" << demo_count << "): ";
    separator = "";
    auto is_even = [](long long n) { return n % 2 == 0; };
    for (auto val : Fibonacci(demo_count) | std::views::filter(is_even)) {
        std::cout << separator << val;
        separator = ", ";
    }
    std::cout << "\n";
}

int main() {
    run_tests_and_demonstration();
    return 0;
}
