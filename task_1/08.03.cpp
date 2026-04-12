/*
g++ -std=c++23 -Wall -Wextra 08.03.cpp -o 08.03.out
./08.03.out
*/

#include <cassert>
#include <climits>
#include <cstdint>
#include <limits>

// constants for IEEE 754 32-bit
constexpr int EXPONENT_BIAS   = 127;
constexpr int MANTISSA_BITS   = 23;
constexpr int EXPONENT_BITS   = 8;
constexpr int DENORM_OFFSET   = 126 + MANTISSA_BITS; 

constexpr uint32_t SIGN_MASK      = 1u << 31;
constexpr uint32_t EXPONENT_MASK  = 0xFFu << MANTISSA_BITS;   
constexpr uint32_t MANTISSA_MASK  = (1u << MANTISSA_BITS) - 1;

/////////////////////////////////////////////////////////////////////////////////////////

// floor(log2(x)) for positive int x
int floor_log2_int(int x)
{
    assert(x > 0);

    unsigned int ux = static_cast<unsigned int>(x);
    unsigned int temp = ux;
    int result = 0;

    while (temp >>= 1)
        ++result;

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

// floor(log2(x)) for positive float x
int floor_log2_float(float x)
{
    assert(x > 0.0f);

    // Extract bits using a union
    union
    {
        float f;
        uint32_t u;
    } converter;

    converter.f = x;
    uint32_t bits = converter.u;

    // Sign must be positive
    uint32_t sign = bits >> 31;
    assert(sign == 0);

    int exponent = (bits >> MANTISSA_BITS) & 0xFF;
    uint32_t mantissa = bits & MANTISSA_MASK;

    // infinity and NaN are not allowed
    if (exponent == 0xFF)
    {
        assert(false);
        return INT_MAX;
    }

    // Zero or denormalized number
    if (exponent == 0)
    {
        assert(mantissa != 0);

        // Find the most significant bit in the mantissa
        uint32_t m = mantissa;
        int pos = -1;
        while (m)
        {
            m >>= 1;
            ++pos;
        }

        // Denormalized value: mantissa * 2^(-149)
        // floor(log2(denorm)) = floor(log2(mantissa)) - 149
        return pos - DENORM_OFFSET;
    }

    return exponent - EXPONENT_BIAS;
}

/////////////////////////////////////////////////////////////////////////////////////////

void test_int()
{
    assert(floor_log2_int(1) == 0);
    assert(floor_log2_int(2) == 1);
    assert(floor_log2_int(3) == 1);
    assert(floor_log2_int(4) == 2);
    assert(floor_log2_int(5) == 2);
    assert(floor_log2_int(7) == 2);
    assert(floor_log2_int(8) == 3);

    assert(floor_log2_int(INT_MAX) == 30);
}

void test_float()
{
    assert(floor_log2_float(1.0f) == 0);
    assert(floor_log2_float(2.0f) == 1);
    assert(floor_log2_float(0.5f) == -1);
    assert(floor_log2_float(0.75f) == -1);

	union
    {
        float f;
        uint32_t u;
    } maker;

    // Smallest positive normalized number 2^-126
    maker.u = 0x00800000;   // 0 00000001 000...
    assert(floor_log2_float(maker.f) == -126);

    // Largest normalized number
    maker.u = 0x7F7FFFFF;   // 0 11111110 111... (exponent 254 → 127)
    assert(floor_log2_float(maker.f) == 127);

    // Smallest positive denormalized (2^-149)
    maker.u = 0x00000001;  
    assert(floor_log2_float(maker.f) == -149);

    // Denormalized with mantissa = 2^22 (0.5 in fractional part) -> 2^-127
    maker.u = 0x00400000;   
    assert(floor_log2_float(maker.f) == -127);

    // Largest denormalized 
    maker.u = 0x007FFFFF;   
    assert(floor_log2_float(maker.f) == -127);
}

/////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    test_int();
    test_float();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////