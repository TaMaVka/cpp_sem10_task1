// g++ -std=c++23 -Wall -Wextra -Wpedantic 11.04.cpp -o 11.04.out

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

using RootsVariant = std::variant<double, std::pair<double, double>, std::monostate>;
using RootsOptional = std::optional<RootsVariant>;

constexpr double kEpsilon = 1e-7;
constexpr double kFormulaFour = 4.0;
constexpr double kFormulaTwo = 2.0;

RootsOptional solve(double a, double b, double c)
{
    if (std::abs(a) < kEpsilon)
    {
        if (std::abs(b) < kEpsilon && std::abs(c) < kEpsilon)
        {
            return std::monostate{};
        }

        throw std::invalid_argument("Error: Not a quadratic equation.");
    }

    const double discriminant = b * b - kFormulaFour * a * c;

    if (discriminant < -kEpsilon)
    {
        return std::nullopt;
    }
    else if (std::abs(discriminant) <= kEpsilon)
    {
        return -b / (kFormulaTwo * a);
    }
    else
    {
        const double root1 = (-b + std::sqrt(discriminant)) / (kFormulaTwo * a);
        const double root2 = (-b - std::sqrt(discriminant)) / (kFormulaTwo * a);

        return std::make_pair(root1, root2);
    }
}

class Visitor
{
public:
    std::string operator()(std::monostate) const
    {
        return "Infinite roots";
    }

    std::string operator()(double root) const
    {
        return "1 root: " + std::to_string(root);
    }

    std::string operator()(const std::pair<double, double>& roots) const
    {
        return "2 roots: " + std::to_string(roots.first) +
               ", " + std::to_string(roots.second);
    }
};

class TestCase
{
public:
    TestCase(double a, double b, double c)
        : m_a(a)
        , m_b(b)
        , m_c(c)
    {
    }

    void demonstrate() const
    {
        std::cout << "Equation: " << m_a << "x^2 + "
                  << m_b << "x + " << m_c << " = 0 -> ";

        try
        {
            RootsOptional const result = solve(m_a, m_b, m_c);

            if (!result.has_value())
            {
                std::cout << "No real roots\n";
            }
            else
            {
                const Visitor visitor;
                std::cout << std::visit(visitor, result.value()) << "\n";
            }
        }
        catch (const std::invalid_argument& error)
        {
            std::cout << error.what() << "\n";
        }
    }

private:
    double m_a;
    double m_b;
    double m_c;
};

void verify_test(bool condition)
{
    if (!condition)
    {
        std::cerr << "Test failed\n";
        std::exit(1);
    }
}

void run_tests_and_demonstration()
{
    const Visitor visitor;

    // Test: 0=0 yields monostate (infinite roots)
    RootsOptional const res_degenerate = solve(0.0, 0.0, 0.0);
    verify_test(res_degenerate.has_value());
    verify_test(std::holds_alternative<std::monostate>(res_degenerate.value()));
    verify_test(std::visit(visitor, res_degenerate.value()) == "Infinite roots");

    // Test: 0*x^2 + 0*x + 5 = 0 throws (not quadratic, impossible)
    bool caught_impossible = false;
    try
    {
        solve(0.0, 0.0, 5.0);
    }
    catch (const std::invalid_argument&)
    {
        caught_impossible = true;
    }
    verify_test(caught_impossible);

    // Test: 0*x^2 + 2*x - 4 = 0 throws (not quadratic, linear)
    bool caught_linear = false;
    try
    {
        solve(0.0, 2.0, -4.0);
    }
    catch (const std::invalid_argument&)
    {
        caught_linear = true;
    }
    verify_test(caught_linear);

    // Test: x^2 + 1 = 0 has no real roots (nullopt)
    RootsOptional const res_no_roots = solve(1.0, 0.0, 1.0);
    verify_test(!res_no_roots.has_value());

    // Test: x^2 - 2x + 1 = 0 has one root via visitor
    RootsOptional const res_one = solve(1.0, -2.0, 1.0);
    verify_test(res_one.has_value());
    verify_test(std::holds_alternative<double>(res_one.value()));
    verify_test(std::abs(std::get<double>(res_one.value()) - 1.0) < kEpsilon);
    std::string one_str = std::visit(visitor, res_one.value());
    verify_test(one_str.find("1 root:") == 0);

    // Test: x^2 - 4 = 0 has two roots via visitor
    RootsOptional const res_two = solve(1.0, 0.0, -4.0);
    verify_test(res_two.has_value());
    verify_test(std::holds_alternative<std::pair<double, double>>(res_two.value()));
    std::string two_str = std::visit(visitor, res_two.value());
    verify_test(two_str.find("2 roots:") == 0);

    std::cout << "All tests passed.\n\n";

    // Demonstration
    const TestCase demos[] =
    {
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 5.0},
        {0.0, 2.0, -4.0},
        {1.0, 0.0, 1.0},
        {1.0, -2.0, 1.0},
        {1.0, 0.0, -4.0}
    };

    for (const auto& demo : demos)
    {
        demo.demonstrate();
    }
}

int main()
{
    run_tests_and_demonstration();
    return 0;
}
