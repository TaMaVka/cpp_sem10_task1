/*
g++ -std=c++23 -Wall -Wextra 05.05.cpp -o 05.05.out
./05.05.out
*/

#include <cassert>
#include <cmath>
#include <compare>
#include <cstdint>
#include <istream>
#include <numeric>
#include <ostream>
#include <sstream>
#include <vector>

// -----------------------------------------------------------------------------
// Mixin templates 
// -----------------------------------------------------------------------------

// addable: provides binary operator+ in terms of operator+=
template <typename Derived>
struct addable
{
    friend Derived operator+(Derived lhs, const Derived& rhs)
    {
        lhs += rhs;
        return lhs;
    }
};

// subtractable: provides binary operator- in terms of operator-=
template <typename Derived>
struct subtractable
{
    friend Derived operator-(Derived lhs, const Derived& rhs)
    {
        lhs -= rhs;
        return lhs;
    }
};

// multipliable: provides binary operator* in terms of operator*=
template <typename Derived>
struct multipliable
{
    friend Derived operator*(Derived lhs, const Derived& rhs)
    {
        lhs *= rhs;
        return lhs;
    }
};

// dividable: provides binary operator/ in terms of operator/=
template <typename Derived>
struct dividable
{
    friend Derived operator/(Derived lhs, const Derived& rhs)
    {
        lhs /= rhs;
        return lhs;
    }
};

// incrementable: provides postfix operator++ in terms of prefix operator++
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

// decrementable: provides postfix operator-- in terms of prefix operator--
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
// Rational class – now a template
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
    /* explicit */ Rational(T num = 0, T den = 1)
        : m_num(num), m_den(den)
    {
        reduce();
    }

    // Conversion to double
    explicit operator double() const
    {
        return 1.0 * m_num / m_den;
    }

    // Compound assignment
    Rational& operator+=(const Rational& other)
    {
        auto lcm = std::lcm(m_den, other.m_den);
        m_num = m_num * (lcm / m_den) + other.m_num * (lcm / other.m_den);
        m_den = lcm;
        reduce();
        return *this;
    }

    Rational& operator-=(const Rational& other)
    {
        return *this += Rational(-other.m_num, other.m_den);
    }

    Rational& operator*=(const Rational& other)
    {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    Rational& operator/=(const Rational& other)
    {
        return *this *= Rational(other.m_den, other.m_num);
    }

    // Prefix increment/decrement
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

    // Three‑way comparison 
    friend std::strong_ordering operator<=>(const Rational& lhs, const Rational& rhs)
    {
        std::int64_t const left  = static_cast<std::int64_t>(lhs.m_num) * rhs.m_den;
        std::int64_t const right = static_cast<std::int64_t>(rhs.m_num) * lhs.m_den;
        if (left < right) return std::strong_ordering::less;
        if (left > right) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    friend bool operator==(const Rational& lhs, const Rational& rhs)
    {
        return lhs.m_num == rhs.m_num && lhs.m_den == rhs.m_den;
    }

    // Stream I/O
    friend std::istream& operator>>(std::istream& stream, Rational& rational)
    {
        return (stream >> rational.m_num).ignore() >> rational.m_den;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Rational& rational)
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
        auto gcd = std::gcd(m_num, m_den);
        m_num /= gcd;
        m_den /= gcd;
    }

    T m_num = 0;
    T m_den = 1;
};

auto equal(double x, double y, double epsilon = 1e-6)
{
    return std::abs(x - y) < epsilon;
}

int main()
{
    Rational<int> x = 1, y(2, 1);

    // Construction tests
    assert(equal(static_cast<double>(x), 1));
    
    // Increment/decrement
    // prefix from members
    assert((++y) == Rational<int>(+3, 1));
    assert((--y) == Rational<int>(+2, 1));
    // postfix from mixins
    assert((x++) == Rational<int>(+1, 1));
    assert((x--) == Rational<int>(+2, 1));

    // Compound assignment (members)
    assert((x += y) == Rational<int>(+3, 1));
    assert((x -= y) == Rational<int>(+1, 1));
    assert((x *= y) == Rational<int>(+2, 1));
    assert((x /= y) == Rational<int>(+1, 1));
    // Binary arithmetic (from mixins)
    assert((x + y) == Rational<int>(+3, 1));
    assert((x - y) == Rational<int>(-1, 1));
    assert((x * y) == Rational<int>(+2, 1));
    assert((x / y) == Rational<int>(+1, 2));

    // Mixed with int (implicit conversion)
    assert((x += 1) == Rational<int>(+2, 1));
    assert((x +  1) == Rational<int>(+3, 1));
    assert((1 +  y) == Rational<int>(+3, 1));
    assert((1 +  1) == Rational<int>(+2, 1));

    // Comparisons (remain in Rational)
    assert((x <  y) == 0);
    assert((x >  y) == 0);
    assert((x <= y) == 1);
    assert((x >= y) == 1);
    assert((x == y) == 1);
    assert((x != y) == 0);

    // Stream I/O
    std::stringstream stream_1("1/2");
    std::stringstream stream_2;
    stream_1 >> x;
    stream_2 << x;
    assert(stream_2.str() == stream_1.str());
}