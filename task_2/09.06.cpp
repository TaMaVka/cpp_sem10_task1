// g++ -std=c++23 -Wall -Wextra -Wpedantic 09.06.cpp -o 09.06.out

#include <boost/iterator/iterator_facade.hpp>
#include <cassert>
#include <iostream>
#include <iterator>

namespace manual {

class Iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = const int*;
    using reference = const int&;

    Iterator() : prev_(0), curr_(1) {}

    const int& operator*() const {
        return prev_;
    }

    Iterator& operator++() {
        int next = prev_ + curr_;
        prev_ = curr_;
        curr_ = next;
        return *this;
    }

    Iterator operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const Iterator& other) const {
        return prev_ == other.prev_ && curr_ == other.curr_;
    }

private:
    int prev_;
    int curr_;
};

static_assert(std::forward_iterator<Iterator>);

} // namespace manual

namespace facade {

class Iterator
    : public boost::iterator_facade<Iterator, const int,
                                    boost::forward_traversal_tag> {
    friend class boost::iterator_core_access;

public:
    Iterator() : prev_(0), curr_(1) {}

private:
    void increment() {
        int next = prev_ + curr_;
        prev_ = curr_;
        curr_ = next;
    }

    bool equal(const Iterator& other) const {
        return prev_ == other.prev_ && curr_ == other.curr_;
    }

    const int& dereference() const {
        return prev_;
    }

    int prev_;
    int curr_;
};

} // namespace facade

template <typename Iter>
void test_fibonacci_iterator() {
    constexpr int fibonacci[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    constexpr int length = static_cast<int>(std::size(fibonacci));

    Iter it;
    for (int i = 0; i < length; ++i) {
        assert(*it == fibonacci[i]);
        ++it;
    }

    Iter a;
    Iter b;
    assert(a == b);
    assert(!(a != b));

    ++a;
    assert(!(a == b));
    assert(a != b);

    Iter c;
    Iter old = c++;
    assert(*old == 0);
    assert(*c == 1);

    Iter d;
    Iter& ref = ++d;
    assert(&ref == &d);
    assert(*d == 1);
}

template <typename Iter>
void demo(const char* label, Iter it, int count) {
    std::cout << label << ": ";
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            std::cout << ", ";
        }
        std::cout << *it;
        ++it;
    }
    std::cout << "\n";
}

int main() {
    constexpr int demo_count = 15;

    test_fibonacci_iterator<manual::Iterator>();
    std::cout << "Manual iterator: all tests passed.\n";

    test_fibonacci_iterator<facade::Iterator>();
    std::cout << "Facade iterator: all tests passed.\n";

    demo("Manual", manual::Iterator(), demo_count);
    demo("Facade", facade::Iterator(), demo_count);

    return 0;
}