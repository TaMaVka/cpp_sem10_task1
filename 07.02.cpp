#include <cassert>
#include <cmath>
#include <compare>
#include <cstdint>
#include <exception>
#include <iostream>
#include <istream>
#include <numeric>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <variant>
#include <vector>

// -----------------------------------------------------------------------------
// Mixin templates
// -----------------------------------------------------------------------------

template <typename Derived>
struct addable
{
    friend Derived operator+(Derived lhs, Derived const& rhs)
    {
        lhs += rhs;
        return lhs;
    }
};

template <typename Derived>
struct subtractable
{
    friend Derived operator-(Derived lhs, Derived const& rhs)
    {
        lhs -= rhs;
        return lhs;
    }
};

template <typename Derived>
struct multipliable
{
    friend Derived operator*(Derived lhs, Derived const& rhs)
    {
        lhs *= rhs;
        return lhs;
    }
};

template <typename Derived>
struct dividable
{
    friend Derived operator/(Derived lhs, Derived const& rhs)
    {
        lhs /= rhs;
        return lhs;
    }
};

template <typename Derived>
struct incrementable
{
    friend Derived operator++(Derived& lhs, int)
    {
        Derived tmp(lhs);
        ++lhs;
        return tmp;
    }
};

template <typename Derived>
struct decrementable
{
    friend Derived operator--(Derived& lhs, int)
    {
        Derived tmp(lhs);
        --lhs;
        return tmp;
    }
};

// -----------------------------------------------------------------------------
// Custom Exception
// -----------------------------------------------------------------------------

class Exception : public std::exception
{
public:
    explicit Exception(char const* message)
        : m_message(message)
    {
    }

    char const* what() const noexcept override
    {
        return m_message;
    }

private:
    char const* m_message;
};

// -----------------------------------------------------------------------------
// Rational class
// -----------------------------------------------------------------------------

template <typename T>
class Rational
    : public addable<Rational<T>>
    , public subtractable<Rational<T>>
    , public multipliable<Rational<T>>
    , public dividable<Rational<T>>
    , public incrementable<Rational<T>>
    , public decrementable<Rational<T>>
{
public:
    Rational(T num = 0, T den = 1)
        : m_num(num)
        , m_den(den)
    {
        if (m_den == 0)
        {
            throw Exception("Denominator cannot be zero");
        }
        reduce();
    }

    explicit operator double() const
    {
        return 1.0 * m_num / m_den;
    }

    Rational& operator+=(Rational const& other)
    {
        auto const least_common_multiple = std::lcm(m_den, other.m_den);
        m_num = m_num * (least_common_multiple / m_den) + other.m_num * (least_common_multiple / other.m_den);
        m_den = least_common_multiple;
        reduce();
        return *this;
    }

    Rational& operator-=(Rational const& other)
    {
        return *this += Rational(-other.m_num, other.m_den);
    }

    Rational& operator*=(Rational const& other)
    {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    Rational& operator/=(Rational const& other)
    {
        return *this *= Rational(other.m_den, other.m_num);
    }

    Rational& operator++()
    {
        *this += 1;
        return *this;
    }

    Rational& operator--()
    {
        *this -= 1;
        return *this;
    }

    friend std::strong_ordering operator<=>(Rational const& lhs, Rational const& rhs)
    {
        std::int64_t const left = static_cast<std::int64_t>(lhs.m_num) * rhs.m_den;
        std::int64_t const right = static_cast<std::int64_t>(rhs.m_num) * lhs.m_den;
        
        if (left < right)
        {
            return std::strong_ordering::less;
        }
        if (left > right)
        {
            return std::strong_ordering::greater;
        }
        
        return std::strong_ordering::equal;
    }

    friend bool operator==(Rational const& lhs, Rational const& rhs)
    {
        return lhs.m_num == rhs.m_num && lhs.m_den == rhs.m_den;
    }

    friend std::istream& operator>>(std::istream& stream, Rational& rational)
    {
        stream >> rational.m_num;
        stream.ignore();
        stream >> rational.m_den;

        if (rational.m_den == 0)
        {
            throw Exception("Denominator cannot be zero read from stream");
        }

        rational.reduce();
        return stream;
    }

    friend std::ostream& operator<<(std::ostream& stream, Rational const& rational)
    {
        return stream << rational.m_num << '/' << rational.m_den;
    }

private:
    void reduce()
    {
        if (m_den < 0)
        {
            m_num = -m_num;
            m_den = -m_den;
        }
        
        auto const greatest_common_divisor = std::gcd(m_num, m_den);
        m_num /= greatest_common_divisor;
        m_den /= greatest_common_divisor;
    }

    T m_num;
    T m_den;
};

// -----------------------------------------------------------------------------
// Tests and Demonstrations
// -----------------------------------------------------------------------------

bool isEqual(double x, double y, double epsilon = 1e-6)
{
    return std::abs(x - y) < epsilon;
}

void runOriginalTests()
{
    Rational<int> x = 1;
    Rational<int> y(2, 1);

    assert(isEqual(static_cast<double>(x), 1.0));
    
    assert((++y) == Rational<int>(+3, 1));
    assert((--y) == Rational<int>(+2, 1));
    assert((x++) == Rational<int>(+1, 1));
    assert((x--) == Rational<int>(+2, 1));

    assert((x += y) == Rational<int>(+3, 1));
    assert((x -= y) == Rational<int>(+1, 1));
    assert((x *= y) == Rational<int>(+2, 1));
    assert((x /= y) == Rational<int>(+1, 1));
    
    assert((x + y) == Rational<int>(+3, 1));
    assert((x - y) == Rational<int>(-1, 1));
    assert((x * y) == Rational<int>(+2, 1));
    assert((x / y) == Rational<int>(+1, 2));

    assert((x += 1) == Rational<int>(+2, 1));
    assert((x +  1) == Rational<int>(+3, 1));
    assert((1 +  y) == Rational<int>(+3, 1));
    assert((1 +  1) == Rational<int>(+2, 1));

    assert((x <  y) == false);
    assert((x >  y) == false);
    assert((x <= y) == true);
    assert((x >= y) == true);
    assert((x == y) == true);
    assert((x != y) == false);

    std::stringstream stream_1("1/2");
    std::stringstream stream_2;
    stream_1 >> x;
    stream_2 << x;
    assert(stream_2.str() == stream_1.str());
}

int main()
{
    // Run all regular operations and tests first to ensure no regressions
    runOriginalTests();

    // Helper lambda to demonstrate the required try-catch block for various scenarios
    auto demonstrateException =[](auto const& exceptionTrigger, char const* explanation)
    {
        try
        {
            exceptionTrigger();
        }
        catch (std::exception const& exception)
        {
            std::cerr << "Exception caught: " << exception.what() << "\n"
                      << "Explanation: " << explanation << "\n\n";
        }
        catch (...)
        {
            std::cerr << "Unknown exception caught.\n\n";
        }
    };

    // 1. Custom Exception
    demonstrateException([]()
    {
        Rational<int> const bad_rational(1, 0);
        (void)bad_rational;
    }, "Class Exception is generated because the Rational denominator was initialized with 0.");

    // 2. std::bad_alloc
    demonstrateException([]()
    {
        std::size_t volatile memory_size = static_cast<std::size_t>(-1) / sizeof(int) - 1;
        int* ptr = new int[memory_size];
        delete[] ptr;
    }, "std::bad_alloc is generated when dynamic memory allocation via 'new' fails.");

    // 3. std::bad_variant_access
    demonstrateException([]()
    {
        std::variant<int, double> var = 42;
        double const bad_value = std::get<double>(var);
        (void)bad_value;
    }, "std::bad_variant_access is generated when trying to read a variant with the wrong active type.");

    // 4. std::bad_optional_access
    demonstrateException([]()
    {
        std::optional<int> opt;
        int const bad_value = opt.value();
        (void)bad_value;
    }, "std::bad_optional_access is generated when calling value() on an empty std::optional.");

    // 5. std::length_error
    demonstrateException([]()
    {
        std::vector<int> vec;
        vec.reserve(vec.max_size() + 1);
    }, "std::length_error is generated when a container is resized beyond its theoretical maximum capacity.");

    // 6. std::out_of_range
    demonstrateException([]()
    {
        std::vector<int> vec = {1, 2, 3};
        // Access index outside valid range [0, size)
        int const val = vec.at(100);
        (void)val;
    }, "Generated by std::vector::at() when the index is outside the valid range.");

    return 0;
}