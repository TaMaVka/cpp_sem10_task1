// g++ -std=c++23 -Wall -Wextra -Wpedantic 09.07.cpp -o 09.07.out

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <utility>

// Orig Pimpl overhead: heap allocation, pointer indirection, cache misses.
// Inline Pimpl avoids heap, improves cache locality, reduces fragmentation.

class Entity {
public:
    class Implementation;

    Entity();
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&& other) noexcept;
    ~Entity();
    Entity& operator=(Entity&& other) noexcept;

    void test() const;

    Implementation* get();
    const Implementation* get() const;

private:
    alignas(std::max_align_t) std::array<std::byte, 16> m_storage{};
};

class Entity::Implementation {
public:
    bool m_is_valid{true};

    Implementation() = default;
    
    Implementation(Implementation&& other) noexcept : m_is_valid(other.m_is_valid) {
        other.m_is_valid = false;
    }
    
    Implementation& operator=(Implementation&& other) noexcept {
        m_is_valid = other.m_is_valid;
        other.m_is_valid = false;
        return *this;
    }
    
    ~Implementation() = default;
};

Entity::Entity() {
    static_assert(sizeof(Implementation) <= sizeof(m_storage), "Size error");
    static_assert(alignof(Implementation) <= alignof(decltype(m_storage)), "Align error");
    new (m_storage.data()) Implementation();
}

Entity::Entity(Entity&& other) noexcept {
    new (m_storage.data()) Implementation(std::move(*other.get()));
}

Entity::~Entity() {
    std::destroy_at(get());
}

Entity& Entity::operator=(Entity&& other) noexcept {
    if (this != &other) {
        std::destroy_at(get());
        new (m_storage.data()) Implementation(std::move(*other.get()));
    }
    return *this;
}

Entity::Implementation* Entity::get() {
    return std::launder(std::bit_cast<Implementation*>(m_storage.data()));
}

const Entity::Implementation* Entity::get() const {
    return std::launder(std::bit_cast<const Implementation*>(m_storage.data()));
}

void Entity::test() const {
    assert(get()->m_is_valid || !get()->m_is_valid);
}

void run_all_tests() {
    // Test default constructor and verify initial state.
    Entity e1;
    assert(e1.get()->m_is_valid);
    e1.test();

    // Test move constructor. State transfers from e1 to e2.
    Entity e2(std::move(e1));
    assert(e2.get()->m_is_valid);
    assert(!e1.get()->m_is_valid);

    // Test move assignment operator. State transfers from e2 to e3.
    Entity e3;
    e3 = std::move(e2);
    assert(e3.get()->m_is_valid);
    assert(!e2.get()->m_is_valid);
}

int main() {
    run_all_tests();
    std::cout << "All tests passed successfully.\n";
    return 0;
}