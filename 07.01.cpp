#include <iostream>
#include <cmath>
#include <optional>
#include <variant>
#include <utility>
#include <cstdlib>
#include <stdexcept>

// Define types for roots
using RootsVariant = std::variant<double, std::pair<double, double>, std::monostate>;
using RootsOptional = std::optional<RootsVariant>;

// Solves the equation ax^2 + bx + c = 0
RootsOptional solve(double a, double b, double c)
{
    const double epsilon = 1e-7;

    if (std::abs(a) < epsilon)
    {
        if (std::abs(b) < epsilon && std::abs(c) < epsilon)
        {
            // Degenerate case (0 = 0) to satisfy the std::monostate requirement
            return std::monostate{};
        }
        
        // Reject linear and impossible equations
        throw std::invalid_argument("Error: Not a quadratic equation.");
    }

    const double formula_four = 4.0;
    const double formula_two = 2.0;
    const double discriminant = b * b - formula_four * a * c;

    if (discriminant < -epsilon)
    {
        return std::nullopt;
    }
    else if (std::abs(discriminant) <= epsilon)
    {
        return -b / (formula_two * a);
    }
    else
    {
        const double root1 = (-b + std::sqrt(discriminant)) / (formula_two * a);
        const double root2 = (-b - std::sqrt(discriminant)) / (formula_two * a);
        
        return std::make_pair(root1, root2);
    }
}

// Wrapper to store equation parameters and demonstrate output
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
        std::cout << "Equation: " << m_a << "x^2 + " << m_b << "x + " << m_c << " = 0 -> ";

        try
        {
            RootsOptional const result = solve(m_a, m_b, m_c);

            if (!result.has_value())
            {
                std::cout << "No real roots\n";
            }
            else
            {
                RootsVariant const& roots = result.value();

                if (std::holds_alternative<std::monostate>(roots))
                {
                    std::cout << "Infinite roots\n";
                }
                else if (std::holds_alternative<double>(roots))
                {
                    std::cout << "1 root: " << std::get<double>(roots) << "\n";
                }
                else if (std::holds_alternative<std::pair<double, double>>(roots))
                {
                    std::pair<double, double> const& pair = std::get<std::pair<double, double>>(roots);
                    std::cout << "2 roots: " << pair.first << ", " << pair.second << "\n";
                }
            }
        }
        catch (std::invalid_argument const& error)
        {
            std::cout << error.what() << "\n";
        }
    }

private:
    double m_a;
    double m_b;
    double m_c;
};

// Halts execution if a test fails
void verifyTest(bool condition)
{
    if (!condition)
    {
        std::cerr << "Test failed\n";
        std::exit(1);
    }
}

// Automated unit tests
void runTests()
{
    const double epsilon = 1e-7;

    RootsOptional const res1 = solve(0.0, 0.0, 0.0);
    verifyTest(res1.has_value() && std::holds_alternative<std::monostate>(res1.value()));

    bool exception_caught_impossible = false;
    try
    {
        solve(0.0, 0.0, 5.0);
    }
    catch (std::invalid_argument const&)
    {
        exception_caught_impossible = true;
    }
    verifyTest(exception_caught_impossible);

    bool exception_caught_linear = false;
    try
    {
        solve(0.0, 2.0, -4.0);
    }
    catch (std::invalid_argument const&)
    {
        exception_caught_linear = true;
    }
    verifyTest(exception_caught_linear);

    RootsOptional const res4 = solve(1.0, 0.0, 1.0);
    verifyTest(!res4.has_value());

    RootsOptional const res5 = solve(1.0, -2.0, 1.0);
    verifyTest(res5.has_value() && std::holds_alternative<double>(res5.value()));
    verifyTest(std::abs(std::get<double>(res5.value()) - 1.0) < epsilon);

    RootsOptional const res6 = solve(1.0, 0.0, -4.0);
    verifyTest(res6.has_value() && std::holds_alternative<std::pair<double, double>>(res6.value()));
}

int main()
{
    runTests();

    TestCase const test1(0.0, 0.0, 0.0);
    TestCase const test2(0.0, 0.0, 5.0);
    TestCase const test3(0.0, 2.0, -4.0);
    TestCase const test4(1.0, 0.0, 1.0);
    TestCase const test5(1.0, -2.0, 1.0);
    TestCase const test6(1.0, 0.0, -4.0);

    test1.demonstrate();
    test2.demonstrate();
    test3.demonstrate();
    test4.demonstrate();
    test5.demonstrate();
    test6.demonstrate();

    return 0;
}