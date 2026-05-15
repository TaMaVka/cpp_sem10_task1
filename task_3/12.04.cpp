//g++ -std=c++23 -Wall -Wextra -Wpedantic 12.04.cpp -o 12.04.out

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

using namespace std::literals;

auto extract_matches(
    const std::string & text,
    const std::regex  & pattern,
    int submatch_index)
{
    std::vector < std::string > results;

    auto lambda = [&results](const auto & match){ results.push_back(match); };

    std::ranges::for_each
    (
        std::sregex_token_iterator(
            std::cbegin(text), std::cend(text), pattern, { submatch_index }),

        std::sregex_token_iterator(),

        lambda
    );

    return results;
}

constexpr int full_match_index   = 0;
constexpr int domain_group_index = 1;

void run_tests_and_demonstration()
{
    std::regex email_pattern(R"(\w+@(\w+\.\w+))");

    // Test: multiple emails extracted correctly
    {
        auto text = R"(Contact info@example.com or support@test.org)"s;

        auto emails = extract_matches(text, email_pattern, full_match_index);

        assert((emails == std::vector < std::string >
            ({ "info@example.com", "support@test.org" })));
    }

    // Test: domains extracted via capture group
    {
        auto text = R"(Contact info@example.com or support@test.org)"s;

        auto domains = extract_matches(text, email_pattern, domain_group_index);

        assert((domains == std::vector < std::string >
            ({ "example.com", "test.org" })));
    }

    // Test: text without emails yields empty result
    {
        auto text = R"(There are no email addresses here)"s;

        auto emails = extract_matches(text, email_pattern, full_match_index);

        assert(emails.empty());
    }

    // Test: single email surrounded by punctuation
    {
        auto text = R"(Write to admin@server.net!)"s;

        auto emails  = extract_matches(text, email_pattern, full_match_index);
        auto domains = extract_matches(text, email_pattern, domain_group_index);

        assert((emails  == std::vector < std::string > ({ "admin@server.net" })));
        assert((domains == std::vector < std::string > ({ "server.net" })));
    }

    // Test: several emails separated by commas
    {
        auto text = R"(To: alice@mail.com, bob@work.org, carol@uni.edu)"s;

        auto emails  = extract_matches(text, email_pattern, full_match_index);
        auto domains = extract_matches(text, email_pattern, domain_group_index);

        assert((emails == std::vector < std::string >
            ({ "alice@mail.com", "bob@work.org", "carol@uni.edu" })));

        assert((domains == std::vector < std::string >
            ({ "mail.com", "work.org", "uni.edu" })));
    }

    std::cout << "All tests passed successfully." << std::endl;

    // Demonstration

    auto demo_text = R"(
        Dear team,
        Please forward reports to manager@company.com and
        send copies to archive@storage.org as well.
        Best regards, user@domain.net
    )"s;

    auto emails  = extract_matches(demo_text, email_pattern, full_match_index);
    auto domains = extract_matches(demo_text, email_pattern, domain_group_index);

    std::cout << "\nDemonstration:" << std::endl;

    for (std::size_t i = 0; i < emails.size(); ++i)
    {
        std::cout << "  email: " << emails[i]
                  << " | domain: " << domains[i] << std::endl;
    }
}

int main()
{
    run_tests_and_demonstration();
}
