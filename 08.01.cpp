/*
g++ -std=c++23 -Wall -Wextra 08.01.cpp -o 08.01.out
./08.01.out
*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <type_traits>

// Entity_v1 : class with a private int field
class Entity_v1
{
private:
    int m_private;

public:
    explicit Entity_v1(int value) : m_private(value) {}

    int get() const { return m_private; }
};

// Entity_v2 : class with a public int field (identical layout)
class Entity_v2
{
public:
    int m_public;
};

int main()
{
    static_assert(std::is_standard_layout_v<Entity_v1>);
    static_assert(std::is_standard_layout_v<Entity_v2>);
    static_assert(sizeof(Entity_v1) == sizeof(Entity_v2));
    static_assert(std::is_trivially_copyable_v<Entity_v1>);   // for memcpy

    std::cout << "\n=== System A : reinterpret_cast to Entity_v2 reference ===\n";

    Entity_v1 objA(10);
    std::cout << "Initial value  : " << objA.get() << '\n';

    Entity_v2& refA = *reinterpret_cast<Entity_v2*>(
                          reinterpret_cast<char*>(&objA)
                      );
    refA.m_public = 42;

    std::cout << "Modified value : " << objA.get() << '\n';
    assert(objA.get() == 42);

    std::cout << "\n=== System B : overwrite with memcpy ===\n";

    Entity_v1 objB(100);
    std::cout << "Initial value  : " << objB.get() << '\n';

    int newValue = 200;
    std::memcpy(&objB, &newValue, sizeof(int));

    std::cout << "Modified value : " << objB.get() << '\n';
    assert(objB.get() == 200);

    std::cout << "\nAll tests passed.\n";
    return 0;
}