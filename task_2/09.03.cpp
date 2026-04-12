// g++ -std=c++23 -Wall -Wextra -Wpedantic main.cpp -o main.out

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Forward declarations for test functions
void run_tests_and_demonstrations();

// ==========================================================================
// Example 05.01: Builder Pattern with Smart Pointers
// ==========================================================================

namespace Example_05_01
{
    struct Entity
    {
        int x = 0;
        int y = 0;
    };

    // Abstract base class for all builders
    class Builder
    {
    public:
        virtual ~Builder() = default;

        auto make_entity() -> std::unique_ptr<Entity>
        {
            m_entity = std::make_unique<Entity>();
            set_x();
            set_y();
            return std::move(m_entity);
        }

        virtual void set_x() const = 0;
        virtual void set_y() const = 0;

    protected:
        std::unique_ptr<Entity> m_entity = nullptr;
    };

    // Concrete builder for a "Client" type entity
    class Builder_Client : public Builder
    {
    public:
        void set_x() const override { m_entity->x = 10; }
        void set_y() const override { m_entity->y = 20; }
    };

    // Concrete builder for a "Server" type entity
    class Builder_Server : public Builder
    {
    public:
        void set_x() const override { m_entity->x = 100; }
        void set_y() const override { m_entity->y = 200; }
    };

    void run_tests()
    {
        // Test client builder configuration.
        auto client_builder = std::make_unique<Builder_Client>();
        auto client_entity = client_builder->make_entity();
        assert(client_entity != nullptr);
        assert(client_entity->x == 10);
        assert(client_entity->y == 20);

        // Test server builder configuration.
        auto server_builder = std::make_unique<Builder_Server>();
        auto server_entity = server_builder->make_entity();
        assert(server_entity != nullptr);
        assert(server_entity->x == 100);
        assert(server_entity->y == 200);
    }

    void run_demonstration()
    {
        std::cout << "Demonstration: Builder Pattern\n";
        auto builder = std::make_unique<Builder_Client>();
        auto entity = builder->make_entity();
        std::cout << "  Client entity created with x=" << entity->x << ", y=" << entity->y << "\n";
        // Memory is managed automatically by unique_ptr.
    }

} // namespace Example_05_01

// ==========================================================================
// Example 05.03: Abstract Factory Pattern with Smart Pointers
// ==========================================================================

namespace Example_05_03
{
    class Entity
    {
    public:
        virtual ~Entity() = default;
        virtual std::string get_type() const = 0;
    };

    class Client : public Entity 
    {
    public:
        std::string get_type() const override { return "Client"; }
    };

    class Server : public Entity 
    {
    public:
        std::string get_type() const override { return "Server"; }
    };

    class Factory
    {
    public:
        virtual ~Factory() = default;
        virtual auto make_entity() const -> std::unique_ptr<Entity> = 0;
    };

    class Factory_Client : public Factory
    {
    public:
        auto make_entity() const -> std::unique_ptr<Entity> override
        {
            return std::make_unique<Client>();
        }
    };

    class Factory_Server : public Factory
    {
    public:
        auto make_entity() const -> std::unique_ptr<Entity> override
        {
            return std::make_unique<Server>();
        }
    };

    void run_tests()
    {
        // Test that client factory produces a Client object.
        auto client_factory = std::make_unique<Factory_Client>();
        auto client_entity = client_factory->make_entity();
        assert(client_entity != nullptr);
        assert(dynamic_cast<Client*>(client_entity.get()) != nullptr);

        // Test that server factory produces a Server object.
        auto server_factory = std::make_unique<Factory_Server>();
        auto server_entity = server_factory->make_entity();
        assert(server_entity != nullptr);
        assert(dynamic_cast<Server*>(server_entity.get()) != nullptr);
    }

    void run_demonstration()
    {
        std::cout << "Demonstration: Abstract Factory Pattern\n";
        auto factory = std::make_unique<Factory_Server>();
        auto entity = factory->make_entity();
        std::cout << "  Factory created an entity of type: " << entity->get_type() << "\n";
        // Memory for both factory and entity is managed automatically.
    }

} // namespace Example_05_03

// ==========================================================================
// Example 05.04: Prototype Pattern with Smart Pointers
// ==========================================================================

namespace Example_05_04
{
    class Entity
    {
    public:
        virtual ~Entity() = default;
        virtual auto copy() const -> std::unique_ptr<Entity> = 0;
    };

    class Client : public Entity
    {
    public:
        auto copy() const -> std::unique_ptr<Entity> override
        {
            return std::make_unique<Client>(*this);
        }
    };

    class Server : public Entity
    {
    public:
        auto copy() const -> std::unique_ptr<Entity> override
        {
            return std::make_unique<Server>(*this);
        }
    };

    class PrototypeRegistry
    {
    public:
        PrototypeRegistry()
        {
            m_entities.push_back(std::make_unique<Client>());
            m_entities.push_back(std::make_unique<Server>());
        }

        auto make_client() const { return m_entities.at(0)->copy(); }
        auto make_server() const { return m_entities.at(1)->copy(); }

    private:
        std::vector<std::unique_ptr<Entity>> m_entities;
    };

    void run_tests()
    {
        PrototypeRegistry registry;
        
        // Test cloning a client prototype.
        auto client_clone = registry.make_client();
        assert(client_clone != nullptr);
        assert(dynamic_cast<Client*>(client_clone.get()) != nullptr);
        
        // Test cloning a server prototype.
        auto server_clone = registry.make_server();
        assert(server_clone != nullptr);
        assert(dynamic_cast<Server*>(server_clone.get()) != nullptr);
    }

    void run_demonstration()
    {
        std::cout << "Demonstration: Prototype Pattern\n";
        PrototypeRegistry registry;
        auto client_copy = registry.make_client();
        std::cout << "  Created a copy of the client prototype.\n";
        // The copy is automatically deleted when client_copy goes out of scope.
    }

} // namespace Example_05_04

// ==========================================================================
// Example 05.09: Composite Pattern with Smart Pointers
// ==========================================================================

namespace Example_05_09
{
    class Entity
    {
    public:
        virtual ~Entity() = default;
        virtual int test() const = 0;
    };

    class Client : public Entity
    {
    public:
        int test() const override { return 1; }
    };

    class Server : public Entity
    {
    public:
        int test() const override { return 2; }
    };

    class Composite : public Entity
    {
    public:
        void add(std::unique_ptr<Entity> entity)
        {
            m_entities.push_back(std::move(entity));
        }

        int test() const override
        {
            int sum = 0;
            for (const auto& entity : m_entities)
            {
                if (entity)
                {
                    sum += entity->test();
                }
            }
            return sum;
        }

    private:
        std::vector<std::unique_ptr<Entity>> m_entities;
    };

    auto make_composite(std::size_t client_count, std::size_t server_count) -> std::unique_ptr<Composite>
    {
        auto composite = std::make_unique<Composite>();
        for (auto i = 0uz; i < client_count; ++i)
        {
            composite->add(std::make_unique<Client>());
        }
        for (auto i = 0uz; i < server_count; ++i)
        {
            composite->add(std::make_unique<Server>());
        }
        return composite;
    }

    void run_tests()
    {
        // Test a simple composite with a flat structure.
        auto simple_composite = make_composite(2, 3); // 2*1 + 3*2 = 8
        assert(simple_composite->test() == 8);
        
        // Test a nested composite structure.
        auto root = std::make_unique<Composite>();
        root->add(std::make_unique<Client>()); // 1
        root->add(make_composite(1, 1));       // 1*1 + 1*2 = 3
        root->add(make_composite(2, 0));       // 2*1 + 0*2 = 2
        // Total = 1 + 3 + 2 = 6
        assert(root->test() == 6);
    }

    void run_demonstration()
    {
        std::cout << "Demonstration: Composite Pattern\n";
        auto root = std::make_unique<Composite>();

        const std::size_t num_sub_composites = 5;
        for (auto i = 0uz; i < num_sub_composites; ++i)
        {
            // Each sub-composite has 1 client and 1 server (value = 1+2=3).
            root->add(make_composite(1, 1));
        }

        std::unique_ptr<Entity> entity = std::move(root);

        const int expected_value = 15; // 5 * (1 + 2)
        int actual_value = entity->test();
        std::cout << "  Composite test value: " << actual_value << " (Expected: " << expected_value << ")\n";
        assert(actual_value == expected_value);
        // All memory is freed automatically when 'entity' goes out of scope.
    }

} // namespace Example_05_09

// ==========================================================================
// Example 05.13: Observer Pattern with Smart Pointers
// ==========================================================================

namespace Example_05_13
{
    class Observer
    {
    public:
        virtual ~Observer() = default;
        virtual void test(int x) const = 0;
    };
    
    class Entity
    {
    public:
        void add(std::unique_ptr<Observer> observer)
        {
            m_observers.push_back(std::move(observer));
        }

        void set(int x)
        {
            m_x = x;
            notify_all();
        }

    private:
        void notify_all() const
        {
            for (const auto& observer : m_observers)
            {
                if (observer)
                {
                    observer->test(m_x);
                }
            }
        }

        int m_x = 0;
        std::vector<std::unique_ptr<Observer>> m_observers;
    };

    class Client : public Observer
    {
    public:
        void test(int x) const override
        {
            std::cout << "  Client::test : x = " << x << "\n";
        }
    };

    class Server : public Observer
    {
    public:
        void test(int x) const override
        {
            std::cout << "  Server::test : x = " << x << "\n";
        }
    };
    
    class MockObserver : public Observer
    {
    public:
        void test(int x) const override
        {
            last_value = x;
            call_count++;
        }
        
        mutable int last_value = 0;
        mutable int call_count = 0;
    };

    void run_tests()
    {
        Entity entity;
        auto mock = std::make_unique<MockObserver>();
        
        // Get a non-owning pointer to the mock for state verification.
        // This is safe as the mock's lifetime is managed by the entity.
        MockObserver* mock_ptr = mock.get();

        entity.add(std::move(mock));

        // Initial state: no notifications sent yet.
        assert(mock_ptr->call_count == 0);
        
        // After first notification.
        entity.set(42);
        assert(mock_ptr->call_count == 1);
        assert(mock_ptr->last_value == 42);

        // After second notification.
        entity.set(99);
        assert(mock_ptr->call_count == 2);
        assert(mock_ptr->last_value == 99);
    }

    void run_demonstration()
    {
        std::cout << "Demonstration: Observer Pattern\n";
        Entity entity;
        
        entity.add(std::make_unique<Client>());
        entity.add(std::make_unique<Server>());

        const int num_iterations = 2;
        for (auto i = 0; i < num_iterations; ++i)
        {
            std::cout << "  Setting entity value to " << (i + 1) << "...\n";
            entity.set(i + 1);
        }
        // Entity and all its owned observers are cleaned up automatically.
    }

} // namespace Example_05_13


// ==========================================================================
// Main Entry Point
// ==========================================================================
void run_tests_and_demonstrations()
{
    const std::string separator = "--------------------------------------------------------\n";

    Example_05_01::run_demonstration();
    Example_05_01::run_tests();
    std::cout << "Example 05.01 Tests Passed.\n" << separator;

    Example_05_03::run_demonstration();
    Example_05_03::run_tests();
    std::cout << "Example 05.03 Tests Passed.\n" << separator;

    Example_05_04::run_demonstration();
    Example_05_04::run_tests();
    std::cout << "Example 05.04 Tests Passed.\n" << separator;

    Example_05_09::run_demonstration();
    Example_05_09::run_tests();
    std::cout << "Example 05.09 Tests Passed.\n" << separator;

    Example_05_13::run_demonstration();
    Example_05_13::run_tests();
    std::cout << "Example 05.13 Tests Passed.\n" << separator;
}

int main()
{
    try
    {
        run_tests_and_demonstrations();
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