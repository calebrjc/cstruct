#include <assert.h>
#include <stdio.h>

#include "jcsw/binpack.h"

void test_binpack_sizeof()
{
    // Test basic specifiers
    assert(binpack_sizeof("x") == 1); // nul byte
    assert(binpack_sizeof("b") == 1); // int8_t
    assert(binpack_sizeof("B") == 1); // uint8_t
    assert(binpack_sizeof("h") == 2); // int16_t
    assert(binpack_sizeof("H") == 2); // uint16_t
    assert(binpack_sizeof("i") == 4); // int32_t
    assert(binpack_sizeof("I") == 4); // uint32_t
    assert(binpack_sizeof("l") == 4); // int32_t
    assert(binpack_sizeof("L") == 4); // uint32_t
    assert(binpack_sizeof("q") == 8); // int64_t
    assert(binpack_sizeof("Q") == 8); // uint64_t
    assert(binpack_sizeof("f") == 4); // float
    assert(binpack_sizeof("d") == 8); // double

    // Test empty format
    assert(binpack_sizeof("") == 0);

    // Test invalid format
    assert(binpack_sizeof(NULL) == 0); // NULL pointer
    assert(binpack_sizeof("z") == 0);  // Invalid specifier
    assert(binpack_sizeof("A") == 0);  // Invalid specifier

    // Test combinations
    assert(binpack_sizeof("bB") == 2);     // 1 + 1 = 2
    assert(binpack_sizeof("hH") == 4);     // 2 + 2 = 4
    assert(binpack_sizeof("iI") == 8);     // 4 + 4 = 8
    assert(binpack_sizeof("lL") == 8);     // 4 + 4 = 8
    assert(binpack_sizeof("qQ") == 16);    // 8 + 8 = 16
    assert(binpack_sizeof("fd") == 12);    // 4 + 8 = 12
    assert(binpack_sizeof("bHiQd") == 23); // 1 + 2 + 4 + 8 + 8 = 23
    assert(binpack_sizeof("xxbbBB") == 6); // 1 + 1 + 1 + 1 + 1 + 1 = 6

    // Test multipliers with specifiers (new feature)
    assert(binpack_sizeof("3b") == 3);   // 3 × int8_t = 3
    assert(binpack_sizeof("2H") == 4);   // 2 × uint16_t = 4
    assert(binpack_sizeof("4i") == 16);  // 4 × int32_t = 16
    assert(binpack_sizeof("10x") == 10); // 10 × nul byte = 10
    assert(binpack_sizeof("3d") == 24);  // 3 × double = 24

    // Test combinations with multipliers
    assert(binpack_sizeof("2b3B") == 5);  // (2 × int8_t) + (3 × uint8_t) = 5
    assert(binpack_sizeof("3h2i") == 14); // (3 × int16_t) + (2 × int32_t) = 14
    assert(
        binpack_sizeof("2x3b4H2i")
        == 21); // (2 × nul) + (3 × int8_t) + (4 × uint16_t) + (2 × int32_t) = 18
    //
    // // Test multi-digit multipliers
    assert(binpack_sizeof("12b") == 12);   // 12 × int8_t = 12
    assert(binpack_sizeof("25x") == 25);   // 25 × nul byte = 25
    assert(binpack_sizeof("100b") == 100); // 100 × int8_t = 100

    // Test edge cases for multipliers
    assert(binpack_sizeof("0b") == 0);  // 0 × anything = 0
    assert(binpack_sizeof("01h") == 2); // Leading zero in multiplier should work
    assert(binpack_sizeof("1") == 0);   // Multiplier without specifier is invalid
    assert(binpack_sizeof("42") == 0);  // Multiplier without specifier is invalid

    // Test invalid multiplier formats
    assert(binpack_sizeof("-1b") == 0);         // Negative multiplier is invalid
    assert(binpack_sizeof("b3") == 0);          // Multiplier after specifier is invalid
    assert(binpack_sizeof("3b2") == 0);         // Trailing number without specifier is invalid
    assert(binpack_sizeof("2147483648b") == 0); // Multiplier too large (overflow)

    // String specifier tests
    // Note: The exact behavior for string handling wasn't specified,
    // so these are assumptions that might need adjustment

    // Test for special case where 's' might be a special case
    // Depending on implementation, these might need to be revised
    assert(binpack_sizeof("s") > 0);  // String should have some size
    assert(binpack_sizeof("2s") > 0); // 2 strings

    // Additional validation tests
    assert(binpack_sizeof("bhi?") == 0); // Invalid due to '?'
    assert(binpack_sizeof("3z") == 0);   // Invalid specifier with multiplier
    assert(binpack_sizeof("3b2z") == 0); // Valid followed by invalid

    printf("All tests passed!\n");
}

int main(int argc, char **argv)
{
    test_binpack_sizeof();
    return 0;
}
