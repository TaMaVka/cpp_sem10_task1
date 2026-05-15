//g++ -std=c++23 -Wall -Wextra -Wpedantic 12.01.cpp -o 12.01.out

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

constexpr int RUB_PER_USD = 75;
constexpr long double USD_PER_RUB = 1.0L / RUB_PER_USD;

// Converts minor currency units (e.g. kopecks -> cents)
// using the given exchange rate. Both currencies use factor-100 subdivision,
// so the rate applies equally to minor units.
long double convert_minor_units(long double source_units, long double rate) {
    return std::round(source_units * rate);
}

// Formats a value in kopecks as a localized RUB string (international form).
std::string format_as_rub(long double kopecks) {
    std::ostringstream oss;
    oss.imbue(std::locale("ru_RU.utf8"));
    oss << std::showbase << std::put_money(kopecks, true);
    return oss.str();
}

// Parses a localized RUB string (international form) into kopecks.
long double parse_rub(const std::string& input) {
    std::istringstream iss(input);
    iss.imbue(std::locale("ru_RU.utf8"));
    long double kopecks = 0.0L;
    iss >> std::get_money(kopecks, true);
    if (iss.fail()) {
        throw std::runtime_error("Failed to parse RUB monetary value");
    }
    return kopecks;
}

// Formats a value in cents as a localized USD string (international form).
std::string format_as_usd(long double cents) {
    std::ostringstream oss;
    oss.imbue(std::locale("en_US.utf8"));
    oss << std::showbase << std::put_money(cents, true);
    return oss.str();
}

// Full pipeline: parses localized RUB string, converts, returns localized USD string.
std::string rub_to_usd(const std::string& rub_str, long double rate) {
    long double kopecks = parse_rub(rub_str);
    long double cents = convert_minor_units(kopecks, rate);
    return format_as_usd(cents);
}

void run_tests_and_demonstration() {
    // Arithmetic: 75000 kopecks (750.00 RUB) at 1/75 -> 1000 cents (10.00 USD)
    assert(static_cast<long long>(convert_minor_units(75000.0L, USD_PER_RUB)) == 1000LL);

    // Arithmetic: zero converts to zero
    assert(static_cast<long long>(convert_minor_units(0.0L, USD_PER_RUB)) == 0LL);

    // Arithmetic: 7500000 kopecks (75000.00 RUB) -> 100000 cents (1000.00 USD)
    assert(static_cast<long long>(convert_minor_units(7500000.0L, USD_PER_RUB)) == 100000LL);

    // Round-trip: format as RUB then parse back gives same kopecks
    {
        constexpr long double original = 750000.0L;
        std::string formatted = format_as_rub(original);
        long double parsed = parse_rub(formatted);
        assert(static_cast<long long>(original) == static_cast<long long>(parsed));
    }

    // USD format produces non-empty output for valid amount
    assert(!format_as_usd(10000.0L).empty());

    // Full pipeline: RUB formatted string converts to non-empty USD string
    {
        std::string rub = format_as_rub(750000.0L);
        std::string usd = rub_to_usd(rub, USD_PER_RUB);
        assert(!usd.empty());
    }

    // Full pipeline round-trip consistency: same input -> same output
    {
        std::string rub = format_as_rub(4500000.0L);
        std::string usd_first = rub_to_usd(rub, USD_PER_RUB);
        std::string usd_second = rub_to_usd(rub, USD_PER_RUB);
        assert(usd_first == usd_second);
    }

    // Demonstration
    std::cout << "=== RUB -> USD (1 USD = " << RUB_PER_USD << " RUB) ===" << '\n';

    constexpr long double demo_kopecks[] = {
        100.0L,       //     1.00 RUB
        10000.0L,     //   100.00 RUB
        750000.0L,    //  7500.00 RUB
        4500000.0L    // 45000.00 RUB
    };

    for (long double kopecks : demo_kopecks) {
        std::string rub = format_as_rub(kopecks);
        std::string usd = rub_to_usd(rub, USD_PER_RUB);
        std::cout << rub << " -> " << usd << '\n';
    }

    std::cout << "All tests passed." << '\n';
}

int main() {
    run_tests_and_demonstration();
    return 0;
}
