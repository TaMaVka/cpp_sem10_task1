// g++ -std=c++23 -Wall -Wextra -Wpedantic 09.05.cpp -o 09.05.out

#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

template <typename T>
class List
{
private:
    struct Node
    {
        T data = T();
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;
    };

public:
    class Iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        Iterator(const List<T>* list, std::shared_ptr<Node> node = nullptr) 
            : m_list(list), m_node(node) {}

        auto& operator++()
        {
            m_node = m_node->next;
            return *this;
        }

        auto operator++(int)
        {
            auto temp = *this;
            ++(*this);
            return temp;
        }

        auto& operator--()
        {
            if (!m_node) 
            {
                // If this is the end() iterator, move to the tail of the list.
                m_node = m_list->m_tail;
            }
            else
            {
                m_node = m_node->prev.lock();
            }
            return *this;
        }

        auto operator--(int)
        {
            auto temp = *this;
            --(*this);
            return temp;
        }

        auto& operator*() const { return m_node->data; }
        auto operator->() const { return &m_node->data; }

        friend auto operator==(Iterator const& lhs, Iterator const& rhs)
        {
            // Iterators are equal if they point to the same node AND same list.
            return lhs.m_node == rhs.m_node && lhs.m_list == rhs.m_list;
        }

    private:
        const List<T>* m_list; // Pointer to the parent list
        std::shared_ptr<Node> m_node;
    };

    auto begin() const { return Iterator(this, m_head); }
    auto end() const { return Iterator(this); }

    void push_back(T value)
    {
        auto new_node = std::make_shared<Node>();
        new_node->data = value;
        new_node->next = nullptr;

        if (!m_head)
        {
            m_head = new_node;
            m_tail = new_node;
        }
        else
        {
            new_node->prev = m_tail;
            m_tail->next = new_node;
            m_tail = new_node;
        }
    }

private:
    std::shared_ptr<Node> m_head;
    std::shared_ptr<Node> m_tail;
};

struct Point
{
    int x = 0;
    int y = 0;
    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y;
    }
};

void run_tests_and_demonstration()
{
    std::cout << "--- Starting List Demonstration and Tests ---\n" << std::endl;

    // --- Demonstration ---
    std::cout << "Demonstration: Creating a list of integers and iterating..." << std::endl;
    List<int> list;
    const int num_elements_to_add = 5;
    for (int i = 1; i <= num_elements_to_add; ++i)
    {
        list.push_back(i * 10);
    }

    std::cout << "  Forward iteration (range-based for): ";
    for (int element : list)
    {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    std::cout << "  Backward iteration: ";
    if (list.begin() != list.end())
    {
        // This loop now works correctly.
        for (auto it = --list.end();; --it)
        {
            std::cout << *it << " ";
            if (it == list.begin()) break;
        }
    }
    std::cout << "\n" << std::endl;

    // --- Tests ---
    std::cout << "Running tests..." << std::endl;

    // Test 1: Forward iteration correctness
    std::vector<int> expected_forward = {10, 20, 30, 40, 50};
    std::size_t index = 0;
    for (int element : list)
    {
        assert(element == expected_forward[index++]);
    }
    std::cout << "  Test 1 (Forward Iteration): Passed." << std::endl;

    // Test 2: Backward iteration correctness
    std::vector<int> expected_backward = {50, 40, 30, 20, 10};
    index = 0;
    if (list.begin() != list.end())
    {
        for (auto it = --list.end();; --it)
        {
            assert(*it == expected_backward[index++]);
            if (it == list.begin()) break;
        }
    }
    std::cout << "  Test 2 (Backward Iteration): Passed." << std::endl;

    // Test 3: Empty list
    List<int> empty_list;
    assert(empty_list.begin() == empty_list.end());
    std::cout << "  Test 3 (Empty List): Passed." << std::endl;

    // Test 4: Single element list
    List<int> single_element_list;
    single_element_list.push_back(42);
    assert(single_element_list.begin() != single_element_list.end());
    assert(*single_element_list.begin() == 42);
    assert(++single_element_list.begin() == single_element_list.end());
    assert(--single_element_list.end() == single_element_list.begin());
    std::cout << "  Test 4 (Single Element List): Passed." << std::endl;

    // Test 5: Iterator operator->
    List<Point> point_list;
    point_list.push_back({1, 2});
    point_list.push_back({3, 4});
    auto point_it = point_list.begin();
    assert(point_it->x == 1 && point_it->y == 2);
    ++point_it;
    assert(point_it->x == 3 && point_it->y == 4);
    std::cout << "  Test 5 (Iterator operator->): Passed." << std::endl;

    std::cout << "\n--- All tests completed successfully. ---\n" << std::endl;
}


int main()
{
    try
    {
        run_tests_and_demonstration();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "An unknown error occurred." << std::endl;
        return 1;
    }
}