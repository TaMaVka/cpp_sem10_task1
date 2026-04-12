// g++ -std=c++23 -Wall -Wextra -Wpedantic 09.10.cpp -o 09.10.out

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <new>
#include <print>
#include <utility>
#include <vector>

#include <boost/noncopyable.hpp>


class Allocator : private boost::noncopyable
{
public :

    virtual ~Allocator() = default;

    virtual auto allocate(std::size_t size) -> void * = 0;

    virtual void deallocate(void * ptr) = 0;

protected :

    template < typename T >
    auto get(void * x) const -> T *
    {
        return static_cast < T * > (x);
    }

    static inline auto s_alignment = alignof(std::max_align_t);
};


//  ===========================================================================


class Linear_Allocator : public Allocator
{
public :

    Linear_Allocator(std::size_t size) : m_size(size)
    {
        m_begin = operator new(m_size, std::align_val_t(s_alignment));
    }

    ~Linear_Allocator() override
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    auto allocate(std::size_t size) -> void * override
    {
        void * begin = get < std::byte > (m_begin) + m_offset;

        auto free = m_size - m_offset;

        if (begin = std::align(s_alignment, size, begin, free); begin)
        {
            m_offset = m_size - free + size;

            return begin;
        }

        return nullptr;
    }

    void deallocate(void *) override {}

    void show() const
    {
        std::println
        (
            "Linear_Allocator::show : m_size = {} m_begin = {:#018x} m_offset = {:0>4}",

            m_size,

            reinterpret_cast < std::uintptr_t > (m_begin),

            m_offset
        );
    }

private :

    std::size_t m_size   = 0;
    std::size_t m_offset = 0;

    void * m_begin = nullptr;
};


//  ===========================================================================


class Stack_Allocator : public Allocator
{
public :

    Stack_Allocator(std::size_t size) : m_size(size)
    {
        m_begin = operator new(m_size, std::align_val_t(s_alignment));
    }

    ~Stack_Allocator() override
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    auto allocate(std::size_t size) -> void * override
    {
        void * begin =

            get < std::byte > (m_begin) + m_offset + sizeof(header_t);

        auto free = m_size - m_offset - sizeof(header_t);

        if (begin = std::align(s_alignment, size, begin, free); begin)
        {
            auto header = get < header_t >
            (
                get < std::byte > (begin) - sizeof(header_t)
            );

            *header = std::distance
            (
                get < std::byte > (m_begin) + m_offset,

                get < std::byte > (begin)
            );

            m_offset =

                get < std::byte > (begin) -

                get < std::byte > (m_begin) + size;

            return begin;
        }

        return nullptr;
    }

    void deallocate(void * x) override
    {
        auto header = get < header_t >
        (
            get < std::byte > (x) - sizeof(header_t)
        );

        m_offset =

            get < std::byte > (x) -

            get < std::byte > (m_begin) - *header;
    }

    void show() const
    {
        std::println
        (
            "Stack_Allocator::show : m_size = {} m_begin = {:#018x} m_offset = {:0>4}",

            m_size,

            reinterpret_cast < std::uintptr_t > (m_begin),

            m_offset
        );
    }

private :

    using header_t = std::uint8_t;

    std::size_t m_size   = 0;
    std::size_t m_offset = 0;

    void * m_begin = nullptr;
};


//  ===========================================================================


class List_Allocator : public Allocator
{
public :

    List_Allocator(std::size_t size, std::size_t step)

    :   m_size(size), m_step(step)
    {
        assert(m_size % m_step == 0 && m_step >= sizeof(Node));

        make_list();

        m_begin = m_head;
    }

    ~List_Allocator() override
    {
        for (auto list : m_lists)
        {
            operator delete(list, m_size, std::align_val_t(s_alignment));
        }
    }

    auto allocate(std::size_t) -> void * override
    {
        if (!m_head)
        {
            if (m_offset == std::size(m_lists))
            {
                make_list();
            }
            else
            {
                m_head = get < Node > (m_lists[++m_offset - 1]);
            }
        }

        auto node = m_head;

        if (!node->next)
        {
            auto next = get < std::byte > (node) + m_step;

            if (next != get < std::byte > (m_lists[m_offset - 1]) + m_size)
            {
                m_head = get < Node > (next);

                m_head->next = nullptr;
            }
            else
            {
                m_head = m_head->next;
            }
        }
        else
        {
            m_head = m_head->next;
        }

        return node;
    }

    void deallocate(void * x) override
    {
        auto node = get < Node > (x);

        node->next = m_head;

        m_head = node;
    }

    void show() const
    {
        std::println
        (
            "List_Allocator::show : m_size = {} m_step = {} "
            "m_begin = {:#018x} m_head = {:#018x} m_offset = {}",

            m_size, m_step,

            reinterpret_cast < std::uintptr_t > (m_begin),

            reinterpret_cast < std::uintptr_t > (m_head),

            m_offset
        );
    }

private :

    struct Node
    {
        Node * next = nullptr;
    };

    void make_list()
    {
        m_head = get < Node >
        (
            operator new(m_size, std::align_val_t(s_alignment))
        );

        m_head->next = nullptr;

        ++m_offset;

        m_lists.push_back(m_head);
    }

    std::size_t m_size   = 0;
    std::size_t m_step   = 0;
    std::size_t m_offset = 0;

    void * m_begin = nullptr;
    Node * m_head  = nullptr;

    std::vector < void * > m_lists;
};


//  ===========================================================================


class Free_List_Allocator : public Allocator
{
public :

    Free_List_Allocator(std::size_t size) : m_size(size)
    {
        assert(m_size >= sizeof(Node) + 1);

        m_begin = operator new(m_size, std::align_val_t(s_alignment));

        m_head = get < Node > (m_begin);

        m_head->size = m_size - sizeof(Header);

        m_head->next = nullptr;
    }

    ~Free_List_Allocator() override
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    auto allocate(std::size_t size) -> void * override
    {
        void * end  = get < std::byte > (m_begin) + sizeof(Header) + size;
        void * next = end;

        auto free = 2 * alignof(Header);

        if (next = std::align(alignof(Header), sizeof(Header), next, free); next)
        {
            auto padding =

                get < std::byte > (next) - get < std::byte > (end);

            if (auto [current, previous] = find(size + padding); current)
            {
                if (current->size >= size + padding + sizeof(Node) + 1)
                {
                    auto step = sizeof(Header) + size + padding;

                    auto node = get < Node >
                    (
                        get < std::byte > (current) + step
                    );

                    node->size = current->size - step;

                    node->next = current->next;

                    current->next = node;
                }
                else
                {
                    padding += current->size - size - padding;
                }

                if (!previous)
                {
                    m_head = current->next;
                }
                else
                {
                    previous->next = current->next;
                }

                auto header = get < Header > (current);

                header->size = size + padding;

                return get < std::byte > (current) + sizeof(Header);
            }
        }

        return nullptr;
    }

    void deallocate(void * x) override
    {
        auto node = get < Node >
        (
            get < std::byte > (x) - sizeof(Header)
        );

        Node * previous = nullptr;
        Node * current  = m_head;

        while (current)
        {
            if (node < current)
            {
                node->next = current;

                if (!previous)
                {
                    m_head = node;
                }
                else
                {
                    previous->next = node;
                }

                break;
            }

            previous = current;

            current  = current->next;
        }

        if (!current)
        {
            node->next = nullptr;

            if (!previous)
            {
                m_head = node;
            }
            else
            {
                previous->next = node;
            }
        }

        merge(previous, node);
    }

    void show() const
    {
        std::print
        (
            "Free_List_Allocator::show : m_size = {} "
            "m_begin = {:#018x} m_head = {:#018x} ",

            m_size,

            reinterpret_cast < std::uintptr_t > (m_begin),

            reinterpret_cast < std::uintptr_t > (m_head)
        );

        if (m_head && m_head->next)
        {
            std::println
            (
                "m_head->next = {:#018x}",

                reinterpret_cast < std::uintptr_t > (m_head->next)
            );
        }
        else
        {
            std::println("");
        }
    }

private :

    struct Node
    {
        std::size_t size = 0;

        Node * next = nullptr;
    };

    struct alignas(std::max_align_t) Header
    {
        std::size_t size = 0;
    };

    auto find(std::size_t size) const -> std::pair < Node *, Node * >
    {
        Node * current  = m_head;
        Node * previous = nullptr;

        while (current && size > current->size)
        {
            previous = current;

            current  = current->next;
        }

        return std::make_pair(current, previous);
    }

    void merge(Node * previous, Node * node) const
    {
        if
        (
            node->next &&

            get < std::byte > (node) + sizeof(Header) + node->size ==

            get < std::byte > (node->next)
        )
        {
            node->size += sizeof(Header) + node->next->size;

            node->next  = node->next->next;
        }

        if
        (
            previous &&

            get < std::byte > (previous) + sizeof(Header) + previous->size ==

            get < std::byte > (node)
        )
        {
            previous->size += sizeof(Header) + node->size;

            previous->next  = node->next;
        }
    }

    std::size_t m_size = 0;

    void * m_begin = nullptr;

    Node * m_head  = nullptr;
};


//  ===================== Tests and Demonstration =============================


void run_tests_and_demonstration()
{
    // Test: Linear_Allocator returns distinct non-null addresses
    {
        Linear_Allocator allocator(1 << 10);

        auto a = allocator.allocate(16);
        auto b = allocator.allocate(16);

        assert(a != nullptr && b != nullptr && a != b);
    }

    // Test: Linear_Allocator deallocate is a safe no-op
    {
        Linear_Allocator allocator(1 << 10);

        auto a = allocator.allocate(16);

        allocator.deallocate(a);

        auto b = allocator.allocate(16);

        assert(b != nullptr);
    }

    // Test: Stack_Allocator LIFO deallocation reuses same address
    {
        Stack_Allocator allocator(1 << 10);

        allocator.allocate(1);
        allocator.allocate(2);

        auto x = allocator.allocate(4);
        auto y = allocator.allocate(8);

        allocator.deallocate(y);
        allocator.deallocate(x);

        auto z = allocator.allocate(4);

        assert(z == x);
    }

    // Test: List_Allocator deallocated blocks reused in LIFO order
    {
        List_Allocator allocator(32, 8);

        allocator.allocate(0);

        auto x = allocator.allocate(0);
        auto y = allocator.allocate(0);

        allocator.allocate(0);

        allocator.deallocate(x);
        allocator.deallocate(y);

        auto z = allocator.allocate(0);

        assert(z == y);
    }

    // Test: Free_List_Allocator adjacent freed blocks merge for reuse
    {
        Free_List_Allocator allocator(1 << 10);

        allocator.allocate(16);

        auto x = allocator.allocate(16);
        auto y = allocator.allocate(16);

        allocator.allocate(16);

        allocator.deallocate(y);
        allocator.deallocate(x);

        auto z = allocator.allocate(32);

        assert(z == x);
    }

    // Test: all four allocators work through base class pointer
    {
        Linear_Allocator    linear(1 << 10);
        Stack_Allocator     stack(1 << 10);
        List_Allocator      list(128, 16);
        Free_List_Allocator free_list(1 << 10);

        std::vector < Allocator * > allocators =
        {
            &linear, &stack, &list, &free_list
        };

        for (auto allocator : allocators)
        {
            auto p = allocator->allocate(8);

            assert(p != nullptr);

            allocator->deallocate(p);
        }
    }

    // Demonstration: Linear_Allocator state transitions
    {
        std::println("--- Linear_Allocator ---");

        Linear_Allocator allocator(1 << 10);

        allocator.show(); allocator.allocate(1);
        allocator.show(); allocator.allocate(2);
        allocator.show(); allocator.allocate(4);
        allocator.show(); allocator.allocate(8);
        allocator.show();
    }

    // Demonstration: Stack_Allocator state transitions
    {
        std::println("--- Stack_Allocator ---");

        Stack_Allocator allocator(1 << 10);

        allocator.show();          allocator.allocate(1);
        allocator.show();          allocator.allocate(2);
        allocator.show(); auto x = allocator.allocate(4);
        allocator.show(); auto y = allocator.allocate(8);

        allocator.show(); allocator.deallocate(y);
        allocator.show(); allocator.deallocate(x);

        allocator.show(); auto z = allocator.allocate(8);
        allocator.show();

        assert(z == x);
    }

    // Demonstration: List_Allocator state transitions
    {
        std::println("--- List_Allocator ---");

        List_Allocator allocator(32, 8);

        allocator.show();          allocator.allocate(0);
        allocator.show(); auto x = allocator.allocate(0);
        allocator.show(); auto y = allocator.allocate(0);
        allocator.show();          allocator.allocate(0);
        allocator.show();          allocator.allocate(0);

        allocator.show(); allocator.deallocate(x);
        allocator.show(); allocator.deallocate(y);

        allocator.show(); auto z = allocator.allocate(0);
        allocator.show();

        assert(z == y);
    }

    // Demonstration: Free_List_Allocator state transitions
    {
        std::println("--- Free_List_Allocator ---");

        Free_List_Allocator allocator(1 << 10);

        allocator.show();          allocator.allocate(16);
        allocator.show(); auto x = allocator.allocate(16);
        allocator.show(); auto y = allocator.allocate(16);
        allocator.show();          allocator.allocate(16);

        allocator.show(); allocator.deallocate(y);
        allocator.show(); allocator.deallocate(x);

        allocator.show(); auto z = allocator.allocate(32);
        allocator.show();

        assert(z == x);
    }

    // Demonstration: polymorphic usage through base class reference
    {
        std::println("--- Polymorphic usage ---");

        auto demonstrate = []
        (
            const char * name,

            Allocator  & allocator,

            std::size_t  size
        )
        {
            auto a = allocator.allocate(size);
            auto b = allocator.allocate(size);

            std::println("  {} : a = {}, b = {}", name, a, b);

            allocator.deallocate(b);
            allocator.deallocate(a);
        };

        Linear_Allocator    linear(1 << 10);
        Stack_Allocator     stack(1 << 10);
        List_Allocator      list(128, 16);
        Free_List_Allocator free_list(1 << 10);

        demonstrate("Linear_Allocator   ", linear,    16);
        demonstrate("Stack_Allocator    ", stack,     16);
        demonstrate("List_Allocator     ", list,      16);
        demonstrate("Free_List_Allocator", free_list, 16);
    }

    std::println("All tests passed successfully");
}


int main()
{
    run_tests_and_demonstration();
}