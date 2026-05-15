// g++ -std=c++23 -Wall -Wextra -Wpedantic 13.02.cpp -o 13.02.out

#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

// ============================================================
// Token stream
// ============================================================

class Stream
{
public:

    using token_t = std::variant<char, double, std::string>;

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
    return std::holds_alternative<char>(token) ? std::get<char>(token) : '\0';
}

static double compute_factorial(double n)
{
    double result = 1.0;

    for (int i = 1; i <= static_cast<int>(n); ++i)
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
// Stream-based recursive descent calculator
//
// Grammar:
//   Statement  : "set" Name "=" Expression | Expression
//   Expression : Term (('+' | '-') Term)*
//   Term       : Power (('*' | '/' | '%') Power)*
//   Power      : Factorial ('^' Power)?
//   Factorial  : Primary ('!')*
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

    std::unordered_map<std::string, double> m_variables;

    auto statement(Stream & stream) -> double
    {
        auto token = stream.get();

        if (std::holds_alternative<std::string>(token) &&
            std::get<std::string>(token) == "set")
        {
            return declaration(stream);
        }

        stream.put(token);

        return expression(stream);
    }

    auto declaration(Stream & stream) -> double
    {
        auto var_name = std::get<std::string>(stream.get());

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
                case '*': x *= power(stream);              break;
                case '/': x /= power(stream);              break;
                case '%': x = std::fmod(x, power(stream)); break;
                default:  stream.put(t);                   return x;
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

        if (std::holds_alternative<double>(token))
        {
            return std::get<double>(token);
        }

        if (std::holds_alternative<std::string>(token))
        {
            return m_variables.at(std::get<std::string>(token));
        }

        switch (std::get<char>(token))
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
// AST-based calculator
// ============================================================

namespace ast_calc {

struct Node
{
    virtual ~Node() = default;

    virtual auto eval() const -> double = 0;
};

using NodePtr = std::unique_ptr<Node>;

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

                node = std::make_unique<BinaryNode>(
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

                node = std::make_unique<BinaryNode>(
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

            return std::make_unique<BinaryNode>(
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
            node = std::make_unique<FactorialNode>(std::move(node));

            t = m_stream.get();
        }

        m_stream.put(t);

        return node;
    }

    auto parse_primary() -> NodePtr
    {
        auto token = m_stream.get();

        if (std::holds_alternative<double>(token))
        {
            return std::make_unique<NumberNode>(std::get<double>(token));
        }

        char ch = std::get<char>(token);

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
                return std::make_unique<UnaryNode>(ch, parse_primary());
            }
            default:
            {
                return std::make_unique<NumberNode>(0.0);
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
// File I/O: write test data, then read back via std::fstream
// ============================================================

// Line format: MODE | EXPRESSION | EXPECTED
// MODE is "BOTH" (test with both calculators) or "STREAM" (stream_calc only)

static const std::string test_filename = "13.02_test_data.txt";

static void write_test_file()
{
    std::ofstream out(test_filename);

    // -- arithmetic --
    out << "BOTH | 2+3 | 5\n";
    out << "BOTH | 10-4 | 6\n";
    out << "BOTH | 3*7 | 21\n";
    out << "BOTH | 20/4 | 5\n";

    // -- modulo --
    out << "BOTH | 10%3 | 1\n";
    out << "BOTH | 17%5 | 2\n";

    // -- exponentiation --
    out << "BOTH | 2^10 | 1024\n";
    out << "BOTH | 3^0 | 1\n";

    // -- right-associativity of ^: 2^(3^2) = 512 --
    out << "BOTH | 2^3^2 | 512\n";

    // -- factorial --
    out << "BOTH | 0! | 1\n";
    out << "BOTH | 1! | 1\n";
    out << "BOTH | 4! | 24\n";
    out << "BOTH | 5! | 120\n";

    // -- round parentheses --
    out << "BOTH | (2+3)*4 | 20\n";

    // -- square brackets --
    out << "BOTH | [2+3]*4 | 20\n";

    // -- curly brackets --
    out << "BOTH | {2+3}*4 | 20\n";

    // -- mixed brackets: ([{1+2}*3]+4)*2 = 26 --
    out << "BOTH | ([{1+2}*3]+4)*2 | 26\n";

    // -- precedence: * before + --
    out << "BOTH | 2+3*4 | 14\n";

    // -- precedence: ! before ^, 3!^2 = 36 --
    out << "BOTH | 3!^2 | 36\n";

    // -- precedence: ^ before *, 2*3^2 = 18 --
    out << "BOTH | 2*3^2 | 18\n";

    // -- unary minus --
    out << "BOTH | -5+8 | 3\n";

    // -- unary plus --
    out << "BOTH | +5 | 5\n";

    // -- factorial of grouped expression: (2+3)! = 120 --
    out << "BOTH | (2+3)! | 120\n";

    // -- combined: 3! + [2^3] * {10%7} = 6 + 24 = 30 --
    out << "BOTH | 3!+[2^3]*{10%7} | 30\n";

    // -- variable declaration (stream_calc only) --
    out << "STREAM | set x = 10 | 10\n";

    // -- variable usage --
    out << "STREAM | x+5 | 15\n";
    out << "STREAM | x^2 | 100\n";
    out << "STREAM | x%3 | 1\n";
}

struct TestEntry
{
    bool both_calculators = true;

    std::string expression;

    double expected = 0.0;
};

static auto parse_test_line(std::string const & line) -> TestEntry
{
    TestEntry entry;

    auto first_pipe = line.find('|');

    auto second_pipe = line.find('|', first_pipe + 1);

    std::string mode = line.substr(0, first_pipe);

    std::string expr = line.substr(first_pipe + 1, second_pipe - first_pipe - 1);

    std::string value = line.substr(second_pipe + 1);

    // trim whitespace
    auto trim = [](std::string & s)
    {
        std::size_t start = s.find_first_not_of(' ');
        std::size_t end   = s.find_last_not_of(' ');

        s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    };

    trim(mode);
    trim(expr);
    trim(value);

    entry.both_calculators = (mode == "BOTH");
    entry.expression = expr;
    entry.expected = std::stod(value);

    return entry;
}

// ============================================================
// Tests and demonstration
// ============================================================

void run_tests_and_demonstration()
{
    write_test_file();

    std::fstream file(test_filename, std::fstream::in);

    assert(file.is_open());

    stream_calc::Calculator sc;
    ast_calc::Calculator ac;

    std::string line;

    int test_count = 0;

    std::cout << "Reading test data from file: " << test_filename << "\n\n";

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        auto entry = parse_test_line(line);

        double stream_result = sc.evaluate(entry.expression);

        assert(approx_equal(stream_result, entry.expected));

        if (entry.both_calculators)
        {
            double ast_result = ac.evaluate(entry.expression);

            assert(approx_equal(ast_result, entry.expected));

            std::cout << "  [BOTH]   " << entry.expression
                      << " = " << stream_result
                      << " (stream), " << ast_result
                      << " (AST)\n";
        }
        else
        {
            std::cout << "  [STREAM] " << entry.expression
                      << " = " << stream_result << "\n";
        }

        ++test_count;
    }

    file.close();

    std::remove(test_filename.c_str());

    std::cout << "\nAll " << test_count << " tests passed.\n";
}

int main()
{
    run_tests_and_demonstration();
}
