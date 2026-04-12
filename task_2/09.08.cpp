// g++ -std=c++23 -Wall -Wextra -Wpedantic 09.08.cpp -o 09.08.out

#include <cstddef>
#include <new>
#include <print>
#include <cassert>

template <typename D>
class Entity
{
public:
    static void* operator new(std::size_t size)
    {
        std::print("Entity::operator new\n");
        return ::operator new(size);
    }

    static void operator delete(void* x) noexcept
    {
        std::print("Entity::operator delete\n");
        ::operator delete(x);
    }

    static void* operator new[](std::size_t size)
    {
        std::print("Entity::operator new[]\n");
        return ::operator new[](size);
    }

    static void operator delete[](void* x) noexcept
    {
        std::print("Entity::operator delete[]\n");
        ::operator delete[](x);
    }

    static void* operator new(std::size_t size, const std::nothrow_t& nt) noexcept
    {
        std::print("Entity::operator new nothrow\n");
        return ::operator new(size, nt);
    }

    static void operator delete(void* x, const std::nothrow_t& nt) noexcept
    {
        std::print("Entity::operator delete nothrow\n");
        ::operator delete(x, nt);
    }

    static void* operator new[](std::size_t size, const std::nothrow_t& nt) noexcept
    {
        std::print("Entity::operator new[] nothrow\n");
        return ::operator new[](size, nt);
    }

    static void operator delete[](void* x, const std::nothrow_t& nt) noexcept
    {
        std::print("Entity::operator delete[] nothrow\n");
        ::operator delete[](x, nt);
    }

protected:
    Entity() = default;
    ~Entity() = default;
};

class Client : private Entity<Client>
{
public:
    Client() { std::print("Client::Client\n"); }
    ~Client() { std::print("Client::~Client\n"); }

    using Entity<Client>::operator new;
    using Entity<Client>::operator delete;
    using Entity<Client>::operator new[];
    using Entity<Client>::operator delete[];
};

void run_tests_and_demonstration()
{
    const std::size_t array_size = 3;

    // Test standard single object allocation
    Client* single_client = new Client;
    assert(single_client != nullptr);
    delete single_client;

    // Test standard array allocation
    Client* array_client = new Client[array_size];
    assert(array_client != nullptr);
    delete[] array_client;

    // Test nothrow single object allocation
    Client* single_nothrow = new (std::nothrow) Client;
    assert(single_nothrow != nullptr);
    delete single_nothrow;

    // Test nothrow array allocation
    Client* array_nothrow = new (std::nothrow) Client[array_size];
    assert(array_nothrow != nullptr);
    delete[] array_nothrow;

    // Explicitly test nothrow delete for single object to ensure accessibility
    void* raw_mem = Client::operator new(sizeof(Client), std::nothrow);
    assert(raw_mem != nullptr);
    Client::operator delete(raw_mem, std::nothrow);

    // Explicitly test nothrow delete for array to ensure accessibility
    void* raw_array_mem = Client::operator new[](sizeof(Client) * array_size, std::nothrow);
    assert(raw_array_mem != nullptr);
    Client::operator delete[](raw_array_mem, std::nothrow);

    std::print("\nAll tests passed successfully.\n");
}

int main()
{
    run_tests_and_demonstration();
    return 0;
}