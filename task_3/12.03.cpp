// g++ -std=c++23 -Wall -Wextra -Wpedantic 12.03.cpp -o 12.03.out

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::string_view longest_palindrome(std::string_view text)
{
    const std::size_t len = text.size();

    if (len <= 1) {
        return text;
    }

    std::vector<bool> table(len * len, false);

    auto at = [len](std::size_t row, std::size_t col) -> std::size_t {
        return row * len + col;
    };

    std::size_t best_start = 0;
    std::size_t best_len   = 1;

    for (std::size_t i = 0; i < len; ++i) {
        table[at(i, i)] = true;
    }

    for (std::size_t i = 0; i + 1 < len; ++i) {
        if (text[i] == text[i + 1]) {
            table[at(i, i + 1)] = true;
            if (best_len < 2) {
                best_start = i;
                best_len   = 2;
            }
        }
    }

    for (std::size_t span = 3; span <= len; ++span) {
        for (std::size_t i = 0; i + span - 1 < len; ++i) {
            std::size_t j = i + span - 1;
            if (text[i] == text[j] && table[at(i + 1, j - 1)]) {
                table[at(i, j)] = true;
                if (span > best_len) {
                    best_start = i;
                    best_len   = span;
                }
            }
        }
    }

    return text.substr(best_start, best_len);
}

void run_tests_and_demonstration()
{
    // empty string returns empty
    assert(longest_palindrome("") == "");

    // single character is a palindrome itself
    assert(longest_palindrome("a") == "a");

    // two identical characters form a palindrome
    assert(longest_palindrome("aa") == "aa");

    // two different characters: first char returned
    assert(longest_palindrome("ab").size() == 1);

    // classic example with odd-length palindrome in the middle
    assert(longest_palindrome("babad") == "bab" ||
           longest_palindrome("babad") == "aba");

    // entire string is a palindrome
    assert(longest_palindrome("racecar") == "racecar");

    // even-length palindrome embedded in string
    assert(longest_palindrome("cbbd") == "bb");

    // longest palindrome at the end
    assert(longest_palindrome("aacabdkd") == "dkd" ||
           longest_palindrome("aacabdkd") == "aca");

    // all identical characters
    assert(longest_palindrome("aaaa") == "aaaa");

    // no palindrome longer than 1
    assert(longest_palindrome("abcdef").size() == 1);

    // palindrome spanning almost entire string
    assert(longest_palindrome("xabacabay") == "abacaba");

    // demonstration
    const std::string demo_inputs[] = {
        "babad", "cbbd", "racecar", "forgeeksskeegfor", "abcdef"
    };

    std::cout << "=== Demonstration ===" << std::endl;
    for (const auto& s : demo_inputs) {
        std::cout << "\"" << s << "\" -> \""
                  << longest_palindrome(s) << "\"" << std::endl;
    }

    std::cout << "All tests passed." << std::endl;
}

int main()
{
    run_tests_and_demonstration();
    return 0;
}
