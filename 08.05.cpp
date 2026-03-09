/*
g++ -std=c++23 -Wall -Wextra 08.05.cpp -o 08.05.out
./08.05.out
*/

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <print>
#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////

template < typename D = std::chrono::duration < double > >
class Timer
{
public:
    Timer(std::string const & scope)
        : m_scope(scope)
        , m_running(false)
        , m_measurements()
    {
    }

    ~Timer()
    {
        if (!m_measurements.empty())
            std::print("{} : average = {:.6f} s\n", m_scope, average());
    }

    void start()
    {
        if (!m_running)
        {
            m_start = clock_t::now();
            m_running = true;
        }
    }

    void stop()
    {
        if (m_running)
        {
            auto end = clock_t::now();
            m_measurements.push_back(end - m_start);
            m_running = false;
        }
    }

    double average() const
    {
        if (m_measurements.empty())
            return 0.0;

        std::chrono::duration<double> sum{0.0};
        for (auto const & d : m_measurements)
            sum += d;
        
        return sum.count() / m_measurements.size();
    }

private:
    using clock_t = std::chrono::steady_clock;

    std::string m_scope;
    bool m_running;
    clock_t::time_point m_start;
    std::vector<std::chrono::duration<double>> m_measurements;
};

/////////////////////////////////////////////////////////////////////////////////

auto calculate(std::size_t size)
{
    auto x = 0.0;

    for (auto i = 0uz; i < size; ++i)
    {
        x += std::pow(std::sin(i), 2) + std::pow(std::cos(i), 2);
    }

    return x;
}

/////////////////////////////////////////////////////////////////////////////////

auto equal(double x, double y, double epsilon = 1e-6)
{
    return std::abs(x - y) < epsilon;
}

/////////////////////////////////////////////////////////////////////////////////

int main()
{
    Timer timer("series of calculations");

    constexpr std::size_t runs = 5;
    constexpr std::size_t workload = 1000000;

    for (std::size_t i = 0; i < runs; ++i)
    {
        timer.start();
        double result = calculate(workload);
        timer.stop();

        assert(equal(result, static_cast<double>(workload)));
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////