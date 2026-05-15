// g++ -std=c++23 -Wall -Wextra -Wpedantic 12.05.cpp -o 12.05.out

#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

// ============================================================
// Token stream: tokenizes input into chars, doubles, or identifiers
// ============================================================

class Stream
{
public:

    using token_t = std::variant < char, double, std::string > ;

    explicit Stream(std::string const & input) : m_stream(input + ';') {}

    auto get() -> token_t
    {
        if (m_has_token)
        {
            m_has_token = false;

            return m_token;
        }

        char ch = '\0';

        m_stream >> ch;

        switch (ch)
        {
            case '+': case '-': case '*': case '/': case '%':
            case '^': case '!': case '=':
            case '(': case ')':
            case '[': case ']':
            case '{': case '}':
            case ';':
                return token_t(ch);

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '.':
            {
                m_stream.unget();

                double value = 0.0;

                m_stream >> value;

                return token_t(value);
            }

            default:
            {
                std::string word(1, ch);

                while (m_stream.get(ch) && (std::isalpha(ch) || std::isdigit(ch)))
                {
                    word += ch;
                }

                if (!m_stream.fail() && !std::isspace(ch))
                {
                    m_stream.unget();
                }

                return token_t(word);
            }
        }
    }

    void put(token_t const & token)
    {
        m_token = token;

        m_has_token = true;
    }

private:

    std::stringstream m_stream;

    token_t m_token;

    bool m_has_token = false;
};

// ============================================================
// Shared utilities
// ============================================================

static char token_char(Stream::token_t const & token)
{
    return std::holds_alternative < char > (token) ? std::get < char > (token) : '\0';
}

static double compute_factorial(double n)
{
    double result = 1.0;

    for (int i = 1; i <= static_cast < int > (n); ++i)
    {
        result *= i;
    }

    return result;
}

constexpr double test_epsilon = 1e-9;

static bool approx_equal(double a, double b)
{
    return std::abs(a - b) < test_epsilon;
}

// ============================================================
// Stream-based recursive descent calculator (based on example 12.19)
//
// Evaluates expressions directly during parsing.
//
// Grammar:
//   Statement  : "set" Name "=" Expression | Expression
//   Expression : Term (('+' | '-') Term)*
//   Term       : Power (('*' | '/' | '%') Power)*
//   Power      : Factorial ('^' Power)?           -- right-associative
//   Factorial  : Primary ('!')*                    -- postfix
//   Primary    : Number | '(' Expr ')' | '[' Expr ']'
//              | '{' Expr '}' | ('+' | '-') Primary | Name
// ============================================================

namespace stream_calc {

class Calculator
{
public:

    auto evaluate(std::string const & input) -> double
    {
        Stream stream(input);

        return statement(stream);
    }

private:

    std::unordered_map < std::string, double > m_variables;

    auto statement(Stream & stream) -> double
    {
        auto token = stream.get();

        if (std::holds_alternative < std::string > (token) &&
            std::get < std::string > (token) == "set")
        {
            return declaration(stream);
        }

        stream.put(token);

        return expression(stream);
    }

    auto declaration(Stream & stream) -> double
    {
        auto var_name = std::get < std::string > (stream.get());

        stream.get();

        double value = expression(stream);

        m_variables[var_name] = value;

        return value;
    }

    auto expression(Stream & stream) -> double
    {
        double x = term(stream);

        for (auto t = stream.get(); ; t = stream.get())
        {
            switch (token_char(t))
            {
                case '+': x += term(stream); break;
                case '-': x -= term(stream); break;
                default:  stream.put(t);     return x;
            }
        }
    }

    auto term(Stream & stream) -> double
    {
        double x = power(stream);

        for (auto t = stream.get(); ; t = stream.get())
        {
            switch (token_char(t))
            {
                case '*': x *= power(stream);                    break;
                case '/': x /= power(stream);                    break;
                case '%': x = std::fmod(x, power(stream));       break;
                default:  stream.put(t);                         return x;
            }
        }
    }

    auto power(Stream & stream) -> double
    {
        double x = factorial_level(stream);

        auto t = stream.get();

        if (token_char(t) == '^')
        {
            return std::pow(x, power(stream));
        }

        stream.put(t);

        return x;
    }

    auto factorial_level(Stream & stream) -> double
    {
        double x = primary(stream);

        auto t = stream.get();

        while (token_char(t) == '!')
        {
            x = compute_factorial(x);

            t = stream.get();
        }

        stream.put(t);

        return x;
    }

    auto primary(Stream & stream) -> double
    {
        auto token = stream.get();

        if (std::holds_alternative < double > (token))
        {
            return std::get < double > (token);
        }

        if (std::holds_alternative < std::string > (token))
        {
            return m_variables.at(std::get < std::string > (token));
        }

        switch (std::get < char > (token))
        {
            case '(':
            {
                double x = expression(stream);
                stream.get();
                return x;
            }
            case '[':
            {
                double x = expression(stream);
                stream.get();
                return x;
            }
            case '{':
            {
                double x = expression(stream);
                stream.get();
                return x;
            }
            case '+': return  primary(stream);
            case '-': return -primary(stream);
            default:  return 0.0;
        }
    }
};

} // namespace stream_calc

// ============================================================
// AST-based calculator (based on example 12.26)
//
// Parses input into an abstract syntax tree, then evaluates.
// Same extended grammar as stream_calc, without variable support.
// ============================================================

namespace ast_calc {

struct Node
{
    virtual ~Node() = default;

    virtual auto eval() const -> double = 0;
};

using NodePtr = std::unique_ptr < Node > ;

struct NumberNode final : Node
{
    double value;

    explicit NumberNode(double v) : value(v) {}

    auto eval() const -> double override { return value; }
};

struct UnaryNode final : Node
{
    char operation;

    NodePtr operand;

    UnaryNode(char op, NodePtr child)
        : operation(op), operand(std::move(child)) {}

    auto eval() const -> double override
    {
        double x = operand->eval();

        return operation == '-' ? -x : x;
    }
};

struct BinaryNode final : Node
{
    char operation;

    NodePtr left;

    NodePtr right;

    BinaryNode(char op, NodePtr l, NodePtr r)
        : operation(op), left(std::move(l)), right(std::move(r)) {}

    auto eval() const -> double override
    {
        double lv = left->eval();

        double rv = right->eval();

        switch (operation)
        {
            case '+': return lv + rv;
            case '-': return lv - rv;
            case '*': return lv * rv;
            case '/': return lv / rv;
            case '%': return std::fmod(lv, rv);
            case '^': return std::pow(lv, rv);
            default:  return 0.0;
        }
    }
};

struct FactorialNode final : Node
{
    NodePtr operand;

    explicit FactorialNode(NodePtr child) : operand(std::move(child)) {}

    auto eval() const -> double override
    {
        return compute_factorial(operand->eval());
    }
};

class Parser
{
public:

    explicit Parser(std::string const & input) : m_stream(input) {}

    auto parse() -> NodePtr { return parse_expression(); }

private:

    Stream m_stream;

    auto parse_expression() -> NodePtr
    {
        auto node = parse_term();

        for (auto t = m_stream.get(); ; t = m_stream.get())
        {
            char ch = token_char(t);

            if (ch == '+' || ch == '-')
            {
                auto right = parse_term();

                node = std::make_unique < BinaryNode > (
                    ch, std::move(node), std::move(right));
            }
            else
            {
                m_stream.put(t);

                return node;
            }
        }
    }

    auto parse_term() -> NodePtr
    {
        auto node = parse_power();

        for (auto t = m_stream.get(); ; t = m_stream.get())
        {
            char ch = token_char(t);

            if (ch == '*' || ch == '/' || ch == '%')
            {
                auto right = parse_power();

                node = std::make_unique < BinaryNode > (
                    ch, std::move(node), std::move(right));
            }
            else
            {
                m_stream.put(t);

                return node;
            }
        }
    }

    auto parse_power() -> NodePtr
    {
        auto node = parse_factorial();

        auto t = m_stream.get();

        if (token_char(t) == '^')
        {
            auto right = parse_power();

            return std::make_unique < BinaryNode > (
                '^', std::move(node), std::move(right));
        }

        m_stream.put(t);

        return node;
    }

    auto parse_factorial() -> NodePtr
    {
        auto node = parse_primary();

        auto t = m_stream.get();

        while (token_char(t) == '!')
        {
            node = std::make_unique < FactorialNode > (std::move(node));

            t = m_stream.get();
        }

        m_stream.put(t);

        return node;
    }

    auto parse_primary() -> NodePtr
    {
        auto token = m_stream.get();

        if (std::holds_alternative < double > (token))
        {
            return std::make_unique < NumberNode > (std::get < double > (token));
        }

        char ch = std::get < char > (token);

        switch (ch)
        {
            case '(': case '[': case '{':
            {
                auto node = parse_expression();

                m_stream.get();

                return node;
            }
            case '+': case '-':
            {
                return std::make_unique < UnaryNode > (ch, parse_primary());
            }
            default:
            {
                return std::make_unique < NumberNode > (0.0);
            }
        }
    }
};

class Calculator
{
public:

    auto evaluate(std::string const & input) -> double
    {
        Parser parser(input);

        auto ast = parser.parse();

        return ast->eval();
    }
};

} // namespace ast_calc

// ============================================================
// Tests and demonstration
// ============================================================

void run_tests_and_demonstration()
{
    stream_calc::Calculator sc;
    ast_calc::Calculator     ac;

    // --- addition ---
    assert(approx_equal(sc.evaluate("2+3"), 5.0));
    assert(approx_equal(ac.evaluate("2+3"), 5.0));

    // --- subtraction ---
    assert(approx_equal(sc.evaluate("10-4"), 6.0));
    assert(approx_equal(ac.evaluate("10-4"), 6.0));

    // --- multiplication ---
    assert(approx_equal(sc.evaluate("3*7"), 21.0));
    assert(approx_equal(ac.evaluate("3*7"), 21.0));

    // --- division ---
    assert(approx_equal(sc.evaluate("20/4"), 5.0));
    assert(approx_equal(ac.evaluate("20/4"), 5.0));

    // --- modulo ---
    assert(approx_equal(sc.evaluate("10%3"), 1.0));
    assert(approx_equal(ac.evaluate("10%3"), 1.0));
    assert(approx_equal(sc.evaluate("17%5"), 2.0));
    assert(approx_equal(ac.evaluate("17%5"), 2.0));

    // --- exponentiation ---
    assert(approx_equal(sc.evaluate("2^10"), 1024.0));
    assert(approx_equal(ac.evaluate("2^10"), 1024.0));
    assert(approx_equal(sc.evaluate("3^0"), 1.0));
    assert(approx_equal(ac.evaluate("3^0"), 1.0));

    // --- right-associativity of ^: 2^3^2 = 2^(3^2) = 512 ---
    assert(approx_equal(sc.evaluate("2^3^2"), 512.0));
    assert(approx_equal(ac.evaluate("2^3^2"), 512.0));

    // --- factorial ---
    assert(approx_equal(sc.evaluate("0!"), 1.0));
    assert(approx_equal(ac.evaluate("0!"), 1.0));
    assert(approx_equal(sc.evaluate("1!"), 1.0));
    assert(approx_equal(ac.evaluate("1!"), 1.0));
    assert(approx_equal(sc.evaluate("4!"), 24.0));
    assert(approx_equal(ac.evaluate("4!"), 24.0));
    assert(approx_equal(sc.evaluate("5!"), 120.0));
    assert(approx_equal(ac.evaluate("5!"), 120.0));

    // --- round parentheses ---
    assert(approx_equal(sc.evaluate("(2+3)*4"), 20.0));
    assert(approx_equal(ac.evaluate("(2+3)*4"), 20.0));

    // --- square brackets ---
    assert(approx_equal(sc.evaluate("[2+3]*4"), 20.0));
    assert(approx_equal(ac.evaluate("[2+3]*4"), 20.0));

    // --- curly brackets ---
    assert(approx_equal(sc.evaluate("{2+3}*4"), 20.0));
    assert(approx_equal(ac.evaluate("{2+3}*4"), 20.0));

    // --- mixed bracket types: ([{1+2}*3]+4)*2 = (9+4)*2 = 26 ---
    assert(approx_equal(sc.evaluate("([{1+2}*3]+4)*2"), 26.0));
    assert(approx_equal(ac.evaluate("([{1+2}*3]+4)*2"), 26.0));

    // --- precedence: * before + ---
    assert(approx_equal(sc.evaluate("2+3*4"), 14.0));
    assert(approx_equal(ac.evaluate("2+3*4"), 14.0));

    // --- precedence: ! before ^, so 3!^2 = 6^2 = 36 ---
    assert(approx_equal(sc.evaluate("3!^2"), 36.0));
    assert(approx_equal(ac.evaluate("3!^2"), 36.0));

    // --- precedence: ^ before *, so 2*3^2 = 18 ---
    assert(approx_equal(sc.evaluate("2*3^2"), 18.0));
    assert(approx_equal(ac.evaluate("2*3^2"), 18.0));

    // --- unary minus ---
    assert(approx_equal(sc.evaluate("-5+8"), 3.0));
    assert(approx_equal(ac.evaluate("-5+8"), 3.0));

    // --- unary plus ---
    assert(approx_equal(sc.evaluate("+5"), 5.0));
    assert(approx_equal(ac.evaluate("+5"), 5.0));

    // --- factorial of grouped expression: (2+3)! = 5! = 120 ---
    assert(approx_equal(sc.evaluate("(2+3)!"), 120.0));
    assert(approx_equal(ac.evaluate("(2+3)!"), 120.0));

    // --- combined: 3! + [2^3] * {10%7} = 6 + 8*3 = 30 ---
    assert(approx_equal(sc.evaluate("3!+[2^3]*{10%7}"), 30.0));
    assert(approx_equal(ac.evaluate("3!+[2^3]*{10%7}"), 30.0));

    // --- variables (stream_calc only) ---
    sc.evaluate("set x = 10");
    assert(approx_equal(sc.evaluate("x+5"), 15.0));
    assert(approx_equal(sc.evaluate("x^2"), 100.0));
    assert(approx_equal(sc.evaluate("x%3"), 1.0));

    // --- demonstration ---
    std::cout << "Demonstration:\n";

    auto demonstrate = [](const char * expr)
    {
        stream_calc::Calculator s;
        ast_calc::Calculator    a;

        double sv = s.evaluate(expr);
        double av = a.evaluate(expr);

        std::cout << "  " << expr << " = " << sv
                  << " (stream), " << av << " (AST)\n";
    };

    demonstrate("2 + 3 * 4");
    demonstrate("(2 + 3) * 4");
    demonstrate("[10 - 2] * {3 + 1}");
    demonstrate("5!");
    demonstrate("2 ^ 10");
    demonstrate("2 ^ 3 ^ 2");
    demonstrate("10 % 3");
    demonstrate("3! ^ 2");
    demonstrate("(2 + 3)!");
    demonstrate("3! + [2^3] * {10%7}");

    std::cout << "All tests passed.\n";
}

int main()
{
    run_tests_and_demonstration();
}
