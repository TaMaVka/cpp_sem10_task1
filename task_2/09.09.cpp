// g++ -std=c++23 -Wall -Wextra -Wpedantic 09.09.cpp -o 09.09.out -lbenchmark -lpthread

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <print>
#include <random>
#include <utility>
#include <vector>

#include <boost/noncopyable.hpp>

#include <benchmark/benchmark.h>


enum class Search_Policy { first_fit, best_fit };


class Allocator : private boost::noncopyable
{
public :

    Allocator
    (
        std::size_t   size,
        Search_Policy policy = Search_Policy::first_fit
    )
    :   m_size(size), m_policy(policy)
    {
        assert(m_size >= sizeof(Node) + 1);

        m_begin = operator new(m_size, std::align_val_t(s_alignment));

        m_head = get_node(m_begin);

        m_head->size = m_size - sizeof(Header);

        m_head->next = nullptr;
    }

//  ---------------------------------------------------------------------------

   ~Allocator()
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

//  ---------------------------------------------------------------------------

    auto allocate(std::size_t size) -> void *
    {
        void * end  = get_byte(m_begin) + sizeof(Header) + size;
        void * next = end;

        auto free = 2 * alignof(Header);

        if (next = std::align(alignof(Header), sizeof(Header), next, free); next)
        {
            auto padding = get_byte(next) - get_byte(end);

            if (auto [current, previous] = find(size + padding); current)
            {
                if (current->size >= size + padding + sizeof(Node) + 1)
                {
                    auto step = sizeof(Header) + size + padding;

                    auto node = get_node(get_byte(current) + step);

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

                auto header = get_header(current);

                header->size = size + padding;

                return get_byte(current) + sizeof(Header);
            }
        }

        return nullptr;
    }

//  ---------------------------------------------------------------------------

    void deallocate(void * x)
    {
        auto node = get_node(get_byte(x) - sizeof(Header));

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

//  ---------------------------------------------------------------------------

    void show() const
    {
        std::print
        (
            "Allocator::show : m_size = {} m_begin = {:#018x} m_head = {:#018x} ",

            m_size,

            reinterpret_cast < std::uintptr_t > (m_begin),

            reinterpret_cast < std::uintptr_t > (m_head)
        );

        if (m_head && m_head->next)
        {
            std::print
            (
                "m_head->next = {:#018x}\n",

                reinterpret_cast < std::uintptr_t > (m_head->next)
            );
        }
        else
        {
            std::print("\n");
        }
    }

private :

    struct Node
    {
        std::size_t size = 0;

        Node * next = nullptr;
    };

//  ---------------------------------------------------------------------------

    struct alignas(std::max_align_t) Header
    {
        std::size_t size = 0;
    };

//  ---------------------------------------------------------------------------

    auto get_byte(void * x) const -> std::byte *
    {
        return static_cast < std::byte * > (x);
    }

//  ---------------------------------------------------------------------------

    auto get_node(void * x) const -> Node *
    {
        return static_cast < Node * > (x);
    }

//  ---------------------------------------------------------------------------

    auto get_header(void * x) const -> Header *
    {
        return static_cast < Header * > (x);
    }

//  ---------------------------------------------------------------------------

    auto find_first(std::size_t size) const -> std::pair < Node *, Node * >
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

//  ---------------------------------------------------------------------------

    auto find_best(std::size_t size) const -> std::pair < Node *, Node * >
    {
        Node * current  = m_head;
        Node * previous = nullptr;

        Node * best          = nullptr;
        Node * best_previous = nullptr;

        while (current)
        {
            if (current->size >= size && (!best || current->size < best->size))
            {
                best          = current;
                best_previous = previous;
            }

            previous = current;

            current  = current->next;
        }

        return std::make_pair(best, best_previous);
    }

//  ---------------------------------------------------------------------------

    auto find(std::size_t size) const -> std::pair < Node *, Node * >
    {
        switch (m_policy)
        {
            case Search_Policy::best_fit  : return find_best(size);
            case Search_Policy::first_fit : return find_first(size);
        }

        return find_first(size);
    }

//  ---------------------------------------------------------------------------

    void merge(Node * previous, Node * node) const
    {
        if
        (
            node->next &&

            get_byte(node) + sizeof(Header) + node->size == get_byte(node->next)
        )
        {
            node->size += sizeof(Header) + node->next->size;

            node->next  = node->next->next;
        }

        if
        (
            previous &&

            get_byte(previous) + sizeof(Header) + previous->size == get_byte(node)
        )
        {
            previous->size += sizeof(Header) + node->size;

            previous->next  = node->next;
        }
    }

//  ---------------------------------------------------------------------------

    std::size_t     m_size   = 0;

    void          * m_begin  = nullptr;

    Node          * m_head   = nullptr;

    Search_Policy   m_policy = Search_Policy::first_fit;

//  ---------------------------------------------------------------------------

    static inline auto s_alignment = alignof(std::max_align_t);
};


//  ============================= Benchmarks ==================================


auto constexpr block_count     = 1uz << 10;

auto constexpr allocation_unit = 1uz << 12;

auto constexpr pool_size       = 128uz << 20;


void benchmark_system(benchmark::State & state)
{
    std::uniform_int_distribution distribution(1, 16);

    std::default_random_engine engine;

    std::vector < std::pair < void *, std::size_t > > blocks(block_count);

    for ([[maybe_unused]] auto element : state)
    {
        for (auto i = 0uz; i < block_count; ++i)
        {
            blocks[i].second = distribution(engine) * allocation_unit;

            blocks[i].first  = operator new(blocks[i].second);
        }

        for (auto i = 0uz; i < block_count; i += 32)
        {
            operator delete(blocks[i].first, blocks[i].second);
        }

        for (auto i = 0uz; i < block_count; i += 32)
        {
            blocks[i].second = distribution(engine) * allocation_unit;

            blocks[i].first  = operator new(blocks[i].second);
        }

        for (auto i = 0uz; i < block_count; ++i)
        {
            operator delete(blocks[i].first, blocks[i].second);
        }

        benchmark::DoNotOptimize(blocks);
    }
}


void benchmark_first_fit(benchmark::State & state)
{
    std::uniform_int_distribution distribution(1, 16);

    std::default_random_engine engine;

    std::vector < void * > blocks(block_count, nullptr);

    for ([[maybe_unused]] auto element : state)
    {
        Allocator allocator(pool_size, Search_Policy::first_fit);

        for (auto i = 0uz; i < block_count; ++i)
        {
            blocks[i] = allocator.allocate(distribution(engine) * allocation_unit);
        }

        for (auto i = 0uz; i < block_count; i += 32)
        {
            allocator.deallocate(blocks[i]);
        }

        for (auto i = 0uz; i < block_count; i += 32)
        {
            blocks[i] = allocator.allocate(distribution(engine) * allocation_unit);
        }

        for (auto i = 0uz; i < block_count; ++i)
        {
            allocator.deallocate(blocks[i]);
        }

        benchmark::DoNotOptimize(blocks);
    }
}


void benchmark_best_fit(benchmark::State & state)
{
    std::uniform_int_distribution distribution(1, 16);

    std::default_random_engine engine;

    std::vector < void * > blocks(block_count, nullptr);

    for ([[maybe_unused]] auto element : state)
    {
        Allocator allocator(pool_size, Search_Policy::best_fit);

        for (auto i = 0uz; i < block_count; ++i)
        {
            blocks[i] = allocator.allocate(distribution(engine) * allocation_unit);
        }

        for (auto i = 0uz; i < block_count; i += 32)
        {
            allocator.deallocate(blocks[i]);
        }

        for (auto i = 0uz; i < block_count; i += 32)
        {
            blocks[i] = allocator.allocate(distribution(engine) * allocation_unit);
        }

        for (auto i = 0uz; i < block_count; ++i)
        {
            allocator.deallocate(blocks[i]);
        }

        benchmark::DoNotOptimize(blocks);
    }
}


BENCHMARK(benchmark_system);

BENCHMARK(benchmark_first_fit);

BENCHMARK(benchmark_best_fit);


//  ===================== Tests and Demonstration =============================


void run_tests_and_demonstration()
{
    // Test: basic allocate and deallocate with first_fit
    {
        Allocator allocator(1 << 10, Search_Policy::first_fit);

        auto p = allocator.allocate(16);

        assert(p != nullptr);

        allocator.deallocate(p);
    }

    // Test: basic allocate and deallocate with best_fit
    {
        Allocator allocator(1 << 10, Search_Policy::best_fit);

        auto p = allocator.allocate(16);

        assert(p != nullptr);

        allocator.deallocate(p);
    }

    // Test: default policy parameter is first_fit
    {
        Allocator allocator(1 << 10);

        auto p = allocator.allocate(16);

        assert(p != nullptr);

        allocator.deallocate(p);
    }

    // Test: consecutive allocations yield distinct addresses
    {
        Allocator allocator(1 << 10, Search_Policy::first_fit);

        auto a = allocator.allocate(16);
        auto b = allocator.allocate(16);

        assert(a != nullptr && b != nullptr && a != b);

        allocator.deallocate(b);
        allocator.deallocate(a);
    }

    // Test: adjacent freed blocks merge for reuse (first_fit)
    {
        Allocator allocator(1 << 10, Search_Policy::first_fit);

        allocator.allocate(16);

        auto x = allocator.allocate(16);
        auto y = allocator.allocate(16);

        allocator.allocate(16);

        allocator.deallocate(y);
        allocator.deallocate(x);

        auto z = allocator.allocate(32);

        assert(z == x);
    }

    // Test: adjacent freed blocks merge for reuse (best_fit)
    {
        Allocator allocator(1 << 10, Search_Policy::best_fit);

        allocator.allocate(16);

        auto x = allocator.allocate(16);
        auto y = allocator.allocate(16);

        allocator.allocate(16);

        allocator.deallocate(y);
        allocator.deallocate(x);

        auto z = allocator.allocate(32);

        assert(z == x);
    }

    // Test: first_fit picks the earliest suitable hole
    {
        Allocator allocator(4096, Search_Policy::first_fit);

        allocator.allocate(16);

        auto b = allocator.allocate(256);

        allocator.allocate(16);

        allocator.allocate(64);

        allocator.allocate(16);

        allocator.deallocate(b);

        auto large_hole = allocator.allocate(32);

        assert(large_hole == b);
    }

    // Test: best_fit picks the smallest suitable hole
    {
        Allocator allocator(4096, Search_Policy::best_fit);

        allocator.allocate(16);

        auto b = allocator.allocate(256);

        allocator.allocate(16);

        auto d = allocator.allocate(64);

        allocator.allocate(16);

        allocator.deallocate(b);
        allocator.deallocate(d);

        auto small_hole = allocator.allocate(32);

        assert(small_hole == d);
    }

    // Test: first_fit and best_fit choose different holes for the same request
    {
        Allocator first_allocator(4096, Search_Policy::first_fit);
        Allocator  best_allocator(4096, Search_Policy::best_fit);

        first_allocator.allocate(16);
         best_allocator.allocate(16);

        auto f_large = first_allocator.allocate(256);
        auto b_large =  best_allocator.allocate(256);

        first_allocator.allocate(16);
         best_allocator.allocate(16);

        auto f_small = first_allocator.allocate(64);
        auto b_small =  best_allocator.allocate(64);

        first_allocator.allocate(16);
         best_allocator.allocate(16);

        first_allocator.deallocate(f_large);
        first_allocator.deallocate(f_small);

         best_allocator.deallocate(b_large);
         best_allocator.deallocate(b_small);

        auto f_result = first_allocator.allocate(32);
        auto b_result =  best_allocator.allocate(32);

        assert(f_result == f_large);
        assert(b_result == b_small);
    }

    // Demonstration: allocator state transitions with show()
    {
        Allocator allocator(1 << 10);

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

    std::println("All tests passed successfully");
}


int main(int argc, char ** argv)
{
    run_tests_and_demonstration();

    benchmark::Initialize(&argc, argv);

    benchmark::RunSpecifiedBenchmarks();
}
