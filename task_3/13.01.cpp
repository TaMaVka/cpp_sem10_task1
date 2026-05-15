// g++ -std=c++23 -Wall -Wextra -Wpedantic 13.01.cpp -o 13.01.out

#include <cassert>
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

template <typename Iter>
std::string bytes_to_hexstr(Iter begin, Iter end)
{
    std::ostringstream oss;
    for (; begin != end; ++begin)
    {
        oss << std::hex << std::right
            << std::setw(2) << std::setfill('0')
            << static_cast<int>(*begin);
    }
    return oss.str();
}

template <typename C>
std::string bytes_to_hexstr(C const & c)
{
    return bytes_to_hexstr(std::cbegin(c), std::cend(c));
}

static std::uint8_t hex_char_to_nibble(char ch)
{
    const char zero = '0';
    const char nine = '9';
    const char a_lower = 'a';
    const char f_lower = 'f';

    if (ch >= zero && ch <= nine)
        return static_cast<std::uint8_t>(ch - zero);
    if (ch >= a_lower && ch <= f_lower)
        return static_cast<std::uint8_t>(ch - a_lower + 10);

    throw std::invalid_argument("invalid hex character");
}

std::vector<std::uint8_t> hexstr_to_bytes(std::string const & hexstr)
{
    if (hexstr.size() % 2 != 0)
        throw std::invalid_argument("hex string length must be even");

    const int nibble_shift = 4;
    std::vector<std::uint8_t> result;
    result.reserve(hexstr.size() / 2);

    for (std::size_t i = 0; i < hexstr.size(); i += 2)
    {
        std::uint8_t high = hex_char_to_nibble(hexstr[i]);
        std::uint8_t low  = hex_char_to_nibble(hexstr[i + 1]);
        result.push_back(static_cast<std::uint8_t>((high << nibble_shift) | low));
    }
    return result;
}

static void print_bytes(std::vector<std::uint8_t> const & v)
{
    std::cout << "{ ";
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        std::cout << "0x" << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0')
                  << static_cast<int>(v[i]);
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << std::dec << std::nouppercase << " }";
}

void run_tests_and_demonstration()
{
    // --- bytes_to_hexstr tests ---

    // well-known pattern "baadf00d"
    std::vector<std::uint8_t> v1{0xBA, 0xAD, 0xF0, 0x0D};
    assert(bytes_to_hexstr(v1) == "baadf00d");

    // sequential small values produce leading zeros
    std::array<std::uint8_t, 6> arr{{1, 2, 3, 4, 5, 6}};
    assert(bytes_to_hexstr(arr) == "010203040506");

    // C-style array support
    std::uint8_t buf[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    assert(bytes_to_hexstr(buf) == "1122334455");

    // empty container yields empty string
    std::vector<std::uint8_t> empty_vec;
    assert(bytes_to_hexstr(empty_vec).empty());

    // boundary byte values 0x00 and 0xFF
    std::vector<std::uint8_t> bounds{0x00, 0xFF};
    assert(bytes_to_hexstr(bounds) == "00ff");

    // single byte
    std::vector<std::uint8_t> single{0xAB};
    assert(bytes_to_hexstr(single) == "ab");

    // --- hexstr_to_bytes tests ---

    // decode known hex string
    auto decoded1 = hexstr_to_bytes("baadf00d");
    assert(decoded1 == v1);

    // decode string with leading zeros
    auto decoded2 = hexstr_to_bytes("010203040506");
    std::vector<std::uint8_t> expected2{1, 2, 3, 4, 5, 6};
    assert(decoded2 == expected2);

    // empty string yields empty vector
    assert(hexstr_to_bytes("").empty());

    // boundary values roundtrip
    auto decoded3 = hexstr_to_bytes("00ff");
    assert(decoded3 == bounds);

    // odd-length string throws
    bool caught_odd = false;
    try { hexstr_to_bytes("abc"); }
    catch (std::invalid_argument const &) { caught_odd = true; }
    assert(caught_odd);

    // invalid character throws
    bool caught_invalid = false;
    try { hexstr_to_bytes("zz"); }
    catch (std::invalid_argument const &) { caught_invalid = true; }
    assert(caught_invalid);

    // full roundtrip: encode then decode returns original
    std::vector<std::uint8_t> original{0xDE, 0xAD, 0xBE, 0xEF};
    assert(hexstr_to_bytes(bytes_to_hexstr(original)) == original);

    // --- demonstration ---

    std::cout << "=== bytes_to_hexstr demonstration ===\n";

    std::cout << "vector {0xBA,0xAD,0xF0,0x0D}  -> \""
              << bytes_to_hexstr(v1) << "\"\n";

    std::cout << "array  {1,2,3,4,5,6}          -> \""
              << bytes_to_hexstr(arr) << "\"\n";

    std::cout << "buf    {0x11..0x55}            -> \""
              << bytes_to_hexstr(buf) << "\"\n";

    std::cout << "vector {0x00,0xFF}             -> \""
              << bytes_to_hexstr(bounds) << "\"\n";

    std::cout << "vector {0xAB}                  -> \""
              << bytes_to_hexstr(single) << "\"\n";

    std::cout << "empty vector                   -> \""
              << bytes_to_hexstr(empty_vec) << "\"\n";

    std::cout << "\n=== hexstr_to_bytes demonstration ===\n";

    auto demo_decode = [](std::string const & hex)
    {
        std::cout << "\"" << hex << "\" -> ";
        print_bytes(hexstr_to_bytes(hex));
        std::cout << "\n";
    };

    demo_decode("baadf00d");
    demo_decode("010203040506");
    demo_decode("1122334455");
    demo_decode("00ff");
    demo_decode("deadbeef");

    std::cout << "\n=== roundtrip demonstration ===\n";
    std::cout << "original:  ";
    print_bytes(original);
    std::string encoded = bytes_to_hexstr(original);
    std::cout << "\nencoded:   \"" << encoded << "\"";
    auto roundtripped = hexstr_to_bytes(encoded);
    std::cout << "\ndecoded:   ";
    print_bytes(roundtripped);
    std::cout << "\nmatch:     " << (roundtripped == original ? "yes" : "no") << "\n";

    std::cout << "\n=== error handling demonstration ===\n";
    std::cout << "hexstr_to_bytes(\"abc\"):  odd length  -> exception caught\n";
    std::cout << "hexstr_to_bytes(\"zz\"):   bad chars   -> exception caught\n";

    std::cout << "\nAll tests passed.\n";
}

int main()
{
    run_tests_and_demonstration();
    return 0;
}
