//g++ -std=c++23 -Wall -Wextra -Wpedantic 13.03.cpp -o 13.03.out

#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <print>
#include <sstream>
#include <string>

void delete_comments(std::string & text)
{
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        if (*it == '\'')
        {
            do { ++it; }
            while (it != text.end() && !(*it == '\'' && *std::prev(it) != '\\'));

            if (it == text.end()) break;
            continue;
        }

        if (*it == 'R' && std::next(it) != text.end() && *std::next(it) == '\"')
        {
            auto pos = std::next(it, 2);
            std::string delimiter;

            while (pos != text.end() && *pos != '(')
            {
                delimiter += *pos;
                ++pos;
            }

            if (pos == text.end()) break;
            ++pos;

            auto closing = ")" + delimiter + "\"";
            auto length  = static_cast<std::string::difference_type>(closing.size());

            while (pos != text.end())
            {
                if (std::distance(pos, text.end()) >= length &&
                    std::string(pos, pos + length) == closing)
                {
                    it = pos + length - 1;
                    break;
                }
                ++pos;
            }

            if (pos == text.end()) break;
            continue;
        }

        if (*it == '\"')
        {
            do { ++it; }
            while (it != text.end() && !(*it == '\"' && *std::prev(it) != '\\'));

            if (it == text.end()) break;
            continue;
        }

        if (*it == '/')
        {
            if (std::next(it) != text.end() && *std::next(it) == '/')
            {
                auto end = std::next(it, 2);

                while (end != text.end() && *end != '\n')
                {
                    ++end;
                }

                it = text.erase(it, end);
            }
            else if (std::next(it) != text.end() && *std::next(it) == '*')
            {
                auto end = std::next(it, 3);

                while (end != text.end() && !(*end == '/' && *std::prev(end) == '*'))
                {
                    ++end;
                }

                if (end != text.end()) ++end;

                it = text.erase(it, end);
            }
        }

        if (it == text.end()) break;
    }
}

std::string remove_blank_lines(std::string const & text)
{
    enum class State { normal, in_char, in_string, in_raw };

    State       state = State::normal;
    std::string raw_closing;
    std::string result;
    std::string line;
    bool        has_content = false;

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        auto c = text[i];
        line += c;

        switch (state)
        {
        case State::in_raw:
        {
            has_content = true;

            if (line.size() >= raw_closing.size())
            {
                auto pos = line.size() - raw_closing.size();

                if (line.substr(pos) == raw_closing)
                {
                    state = State::normal;
                }
            }

            if (c == '\n')
            {
                result += line;
                line.clear();
                has_content = (state == State::in_raw);
            }

            break;
        }
        case State::in_char:
        {
            if (c == '\'' && i > 0 && text[i - 1] != '\\')
            {
                state = State::normal;
            }

            break;
        }
        case State::in_string:
        {
            if (c == '\"' && i > 0 && text[i - 1] != '\\')
            {
                state = State::normal;
            }

            break;
        }
        case State::normal:
        {
            if (c == '\'')
            {
                state       = State::in_char;
                has_content = true;
            }
            else if (c == 'R' && i + 1 < text.size() && text[i + 1] == '\"')
            {
                has_content = true;
                line += text[++i];

                std::string delimiter;

                while (i + 1 < text.size() && text[i + 1] != '(')
                {
                    ++i;
                    delimiter += text[i];
                    line += text[i];
                }

                if (i + 1 < text.size())
                {
                    ++i;
                    line += text[i];
                }

                raw_closing = ")" + delimiter + "\"";
                state       = State::in_raw;
            }
            else if (c == '\"')
            {
                state       = State::in_string;
                has_content = true;
            }
            else if (c == '\n')
            {
                if (has_content)
                {
                    result += line;
                }

                line.clear();
                has_content = false;
            }
            else if (!std::isspace(static_cast<unsigned char>(c)))
            {
                has_content = true;
            }

            break;
        }
        }
    }

    if (has_content && !line.empty())
    {
        result += line;
    }

    return result;
}

std::string transform_string(std::string text)
{
    delete_comments(text);
    return remove_blank_lines(text);
}

void transform_file(std::string const & source, std::string const & target)
{
    std::stringstream buffer;
    buffer << std::fstream(source, std::ios::in).rdbuf();

    std::fstream(target, std::ios::out) << transform_string(buffer.str());
}

void run_tests_and_demonstration()
{
    // single-line comment is removed
    assert(transform_string("int x = 1; // comment\n") == "int x = 1; \n");

    // block comment is removed
    assert(transform_string("int x; /* comment */ int y;\n") == "int x;  int y;\n");

    // line containing only a comment becomes blank and is removed
    assert(transform_string("int x;\n// comment\nint y;\n") == "int x;\nint y;\n");

    // whitespace-only line is removed
    assert(transform_string("int x;\n   \t  \nint y;\n") == "int x;\nint y;\n");

    // empty line is removed
    assert(transform_string("int x;\n\nint y;\n") == "int x;\nint y;\n");

    // comment-like content in regular string literal is preserved
    assert(transform_string("auto s = \"// not a comment\";\n") ==
                            "auto s = \"// not a comment\";\n");

    // slash in char literal does not start a comment
    assert(transform_string("char c = '/';\nint x;\n") == "char c = '/';\nint x;\n");

    // comment-like content in raw string literal is preserved
    assert(transform_string("auto s = R\"(// not a comment)\";\n") ==
                            "auto s = R\"(// not a comment)\";\n");

    // raw string literal with custom delimiter preserves content
    assert(transform_string("auto s = R\"d(/* not */)d\";\n") ==
                            "auto s = R\"d(/* not */)d\";\n");

    // blank lines inside multiline raw string are preserved
    assert(transform_string("auto s = R\"(a\n\nb)\";\n") ==
                            "auto s = R\"(a\n\nb)\";\n");

    // multi-line block comment removal with resulting blank lines
    assert(transform_string("int a;\n/* l1\nl2 */\nint b;\n") == "int a;\nint b;\n");

    // consecutive comment-only lines are all removed
    assert(transform_string("a;\n//c1\n//c2\nb;\n") == "a;\nb;\n");

    // raw string with delimiter containing closing-like subsequence
    assert(transform_string("auto s = R\"x()\")x\";\n") ==
                            "auto s = R\"x()\")x\";\n");

    // file-based demonstration
    auto source_path = std::string("__test_source.cpp");
    auto target_path = std::string("__test_output.cpp");

    std::string sample =
        "#include <iostream>\n"
        "\n"
        "// Main function\n"
        "int main()\n"
        "{\n"
        "    int x = 42; /* the answer */\n"
        "    \n"
        "    auto msg = R\"(has // slashes and\n"
        "\n"
        "blank lines)\";\n"
        "    /* block\n"
        "       comment */\n"
        "    return 0; // exit\n"
        "}\n";

    std::fstream(source_path, std::ios::out) << sample;
    transform_file(source_path, target_path);

    std::stringstream result_buffer;
    result_buffer << std::fstream(target_path, std::ios::in).rdbuf();
    auto result = result_buffer.str();

    std::println("--- Input ---\n{}", sample);
    std::println("--- Output ---\n{}", result);

    std::filesystem::remove(source_path);
    std::filesystem::remove(target_path);

    std::println("All tests passed successfully.");
}

int main()
{
    run_tests_and_demonstration();
}
