// g++ -std=c++23 -Wall -Wextra -Wpedantic 13.04.cpp -o 13.04.out

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <print>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

auto make_type(std::filesystem::file_status const & status)
{
    if (std::filesystem::is_directory   (status)) { return 'd'; }
    if (std::filesystem::is_regular_file(status)) { return 'f'; }
    if (std::filesystem::is_symlink     (status)) { return 'l'; }
    return '?';
}

auto make_permissions(std::filesystem::perms permissions) -> std::string
{
    auto lambda = [permissions](auto x, auto y)
    {
        return (permissions & x) == std::filesystem::perms::none ? '-' : y;
    };

    return
    {
        lambda(std::filesystem::perms::owner_read,  'r'),
        lambda(std::filesystem::perms::owner_write, 'w'),
        lambda(std::filesystem::perms::owner_exec,  'x')
    };
}

auto size(std::filesystem::path const & path)
{
    auto size = 0uz;

    if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
    {
        for (auto const & entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (!std::filesystem::is_directory(entry.status()))
            {
                size += std::filesystem::file_size(entry);
            }
        }
    }

    return size;
}

auto size(std::filesystem::directory_entry const & entry)
{
    auto size = 0uz;

    if (std::filesystem::is_regular_file(entry.status()))
    {
        size = std::filesystem::file_size(entry);
    }
    else
    {
        size = ::size(entry.path());
    }

    std::vector < char > vector = { 'B', 'K', 'M', 'G' };

    auto i = 0uz;

    constexpr auto kilo = 1uz << 10;

    while (i < std::size(vector) && size >= kilo)
    {
        size /= kilo;
        ++i;
    }

    return (std::stringstream() << std::format("{: >4} ({})", size, vector[i])).str();
}

auto matches_pattern(
    std::filesystem::directory_entry const & entry,
    std::regex const & pattern) -> bool
{
    auto filename = entry.path().filename().string();

    return std::regex_search(filename, pattern);
}

void show(std::filesystem::path const & path, std::regex const & pattern)
{
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
    {
        for (auto const & entry : std::filesystem::directory_iterator(path))
        {
            if (matches_pattern(entry, pattern))
            {
                std::print
                (
                    "show : entry : {} | {} | {} | {} | {}\n",

                    make_type(entry.status()),
                    make_permissions(entry.status().permissions()),
                    size(entry),

                    std::chrono::floor < std::chrono::seconds >
                    (
                        std::chrono::file_clock::to_sys(entry.last_write_time())
                    ),

                    entry.path().filename().string()
                );
            }
        }
    }
}

void show(std::filesystem::path const & path)
{
    std::regex match_all(".*");

    show(path, match_all);
}

auto collect_matching(
    std::filesystem::path const & path,
    std::regex const & pattern) -> std::vector < std::string >
{
    std::vector < std::string > result;

    if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
    {
        for (auto const & entry : std::filesystem::directory_iterator(path))
        {
            if (matches_pattern(entry, pattern))
            {
                result.push_back(entry.path().filename().string());
            }
        }
    }

    return result;
}

void create_test_file(std::filesystem::path const & filepath)
{
    std::ofstream stream(filepath);

    stream.put(' ');
}

void run_tests_and_demonstration()
{
    auto passed = 0;
    auto failed = 0;

    auto check = [&passed, &failed](bool condition, std::string const & name)
    {
        if (condition) { ++passed; }
        else
        {
            std::print("FAIL : {}\n", name);
            ++failed;
        }
    };

    // test: regex matches .cpp files among created test files
    {
        auto test_dir = std::filesystem::temp_directory_path() / "test_13_04";

        std::filesystem::create_directories(test_dir);

        create_test_file(test_dir / "alpha.cpp");
        create_test_file(test_dir / "beta.cpp");
        create_test_file(test_dir / "gamma.txt");
        create_test_file(test_dir / "delta.h");

        std::regex cpp_pattern(R"(\.cpp$)");

        auto matches = collect_matching(test_dir, cpp_pattern);

        check(matches.size() == 2, "cpp filter finds exactly 2 files");

        auto has_alpha = false;
        auto has_beta  = false;

        for (auto const & name : matches)
        {
            if (name == "alpha.cpp") { has_alpha = true; }
            if (name == "beta.cpp")  { has_beta  = true; }
        }

        check(has_alpha && has_beta, "cpp filter finds alpha.cpp and beta.cpp");

        std::filesystem::remove_all(test_dir);
    }

    // test: regex matches .txt files only
    {
        auto test_dir = std::filesystem::temp_directory_path() / "test_13_04_txt";

        std::filesystem::create_directories(test_dir);

        create_test_file(test_dir / "notes.txt");
        create_test_file(test_dir / "readme.md");
        create_test_file(test_dir / "log.txt");

        std::regex txt_pattern(R"(\.txt$)");

        auto matches = collect_matching(test_dir, txt_pattern);

        check(matches.size() == 2, "txt filter finds exactly 2 files");

        std::filesystem::remove_all(test_dir);
    }

    // test: pattern matching no files returns empty result
    {
        auto test_dir = std::filesystem::temp_directory_path() / "test_13_04_none";

        std::filesystem::create_directories(test_dir);

        create_test_file(test_dir / "data.csv");

        std::regex xml_pattern(R"(\.xml$)");

        auto matches = collect_matching(test_dir, xml_pattern);

        check(matches.empty(), "xml filter on csv-only dir yields empty");

        std::filesystem::remove_all(test_dir);
    }

    // test: match-all regex returns every entry
    {
        auto test_dir = std::filesystem::temp_directory_path() / "test_13_04_all";

        std::filesystem::create_directories(test_dir);

        create_test_file(test_dir / "one.a");
        create_test_file(test_dir / "two.b");
        create_test_file(test_dir / "three.c");

        std::regex all_pattern(".*");

        auto matches = collect_matching(test_dir, all_pattern);

        check(matches.size() == 3, "match-all regex returns all 3 entries");

        std::filesystem::remove_all(test_dir);
    }

    // test: prefix-based pattern filtering
    {
        auto test_dir = std::filesystem::temp_directory_path() / "test_13_04_prefix";

        std::filesystem::create_directories(test_dir);

        create_test_file(test_dir / "test_main.cpp");
        create_test_file(test_dir / "test_util.cpp");
        create_test_file(test_dir / "main.cpp");

        std::regex prefix_pattern("^test_");

        auto matches = collect_matching(test_dir, prefix_pattern);

        check(matches.size() == 2, "prefix 'test_' matches exactly 2 files");

        std::filesystem::remove_all(test_dir);
    }

    // test: nonexistent directory yields empty result
    {
        std::regex any_pattern(".*");

        auto matches = collect_matching("/nonexistent_path_13_04", any_pattern);

        check(matches.empty(), "nonexistent directory yields empty");
    }

    // test: matches_pattern correctly identifies matching entry
    {
        auto test_dir = std::filesystem::temp_directory_path() / "test_13_04_mp";

        std::filesystem::create_directories(test_dir);

        create_test_file(test_dir / "report.pdf");

        std::filesystem::directory_entry entry(test_dir / "report.pdf");

        std::regex pdf_pattern(R"(\.pdf$)");
        std::regex jpg_pattern(R"(\.jpg$)");

        check(matches_pattern(entry, pdf_pattern),  "entry matches .pdf");
        check(!matches_pattern(entry, jpg_pattern), "entry does not match .jpg");

        std::filesystem::remove_all(test_dir);
    }

    std::print("\nTests : {} passed, {} failed\n\n", passed, failed);

    // demonstration: filtered listing of current directory
    std::print("--- Demonstration: all entries ---\n");
    show(std::filesystem::current_path());

    std::print("\n--- Demonstration: .cpp files only (like grep '\\.cpp$') ---\n");
    std::regex demo_pattern(R"(\.cpp$)");
    show(std::filesystem::current_path(), demo_pattern);
}

int main()
{
    run_tests_and_demonstration();
}