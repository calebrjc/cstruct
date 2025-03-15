#include <assert.h>
#include <stdio.h>

#include "cstruct.h"

void test_cstruct_sizeof()
{
    // Test basic specifiers
    assert(cstruct_sizeof("x") == 1); // nul byte
    assert(cstruct_sizeof("b") == 1); // int8_t
    assert(cstruct_sizeof("B") == 1); // uint8_t
    assert(cstruct_sizeof("h") == 2); // int16_t
    assert(cstruct_sizeof("H") == 2); // uint16_t
    assert(cstruct_sizeof("i") == 4); // int32_t
    assert(cstruct_sizeof("I") == 4); // uint32_t
    assert(cstruct_sizeof("l") == 4); // int32_t
    assert(cstruct_sizeof("L") == 4); // uint32_t
    assert(cstruct_sizeof("q") == 8); // int64_t
    assert(cstruct_sizeof("Q") == 8); // uint64_t
    assert(cstruct_sizeof("f") == 4); // float
    assert(cstruct_sizeof("d") == 8); // double

    // Test empty format
    assert(cstruct_sizeof("") == 0);

    // Test invalid format
    assert(cstruct_sizeof(NULL) == 0); // NULL pointer
    assert(cstruct_sizeof("z") == 0);  // Invalid specifier
    assert(cstruct_sizeof("A") == 0);  // Invalid specifier

    // Test combinations
    assert(cstruct_sizeof("bB") == 2);     // 1 + 1 = 2
    assert(cstruct_sizeof("hH") == 4);     // 2 + 2 = 4
    assert(cstruct_sizeof("iI") == 8);     // 4 + 4 = 8
    assert(cstruct_sizeof("lL") == 8);     // 4 + 4 = 8
    assert(cstruct_sizeof("qQ") == 16);    // 8 + 8 = 16
    assert(cstruct_sizeof("fd") == 12);    // 4 + 8 = 12
    assert(cstruct_sizeof("bHiQd") == 23); // 1 + 2 + 4 + 8 + 8 = 23
    assert(cstruct_sizeof("xxbbBB") == 6); // 1 + 1 + 1 + 1 + 1 + 1 = 6

    // Test multipliers with specifiers (new feature)
    assert(cstruct_sizeof("3b") == 3);   // 3 × int8_t = 3
    assert(cstruct_sizeof("2H") == 4);   // 2 × uint16_t = 4
    assert(cstruct_sizeof("4i") == 16);  // 4 × int32_t = 16
    assert(cstruct_sizeof("10x") == 10); // 10 × nul byte = 10
    assert(cstruct_sizeof("3d") == 24);  // 3 × double = 24

    // Test combinations with multipliers
    assert(cstruct_sizeof("2b3B") == 5);  // (2 × int8_t) + (3 × uint8_t) = 5
    assert(cstruct_sizeof("3h2i") == 14); // (3 × int16_t) + (2 × int32_t) = 14
    assert(
        cstruct_sizeof("2x3b4H2i")
        == 21); // (2 × nul) + (3 × int8_t) + (4 × uint16_t) + (2 × int32_t) = 18
    //
    // // Test multi-digit multipliers
    assert(cstruct_sizeof("12b") == 12);   // 12 × int8_t = 12
    assert(cstruct_sizeof("25x") == 25);   // 25 × nul byte = 25
    assert(cstruct_sizeof("100b") == 100); // 100 × int8_t = 100

    // Test edge cases for multipliers
    assert(cstruct_sizeof("0b") == 0);  // 0 × anything = 0
    assert(cstruct_sizeof("01h") == 2); // Leading zero in multiplier should work
    assert(cstruct_sizeof("1") == 0);   // Multiplier without specifier is invalid
    assert(cstruct_sizeof("42") == 0);  // Multiplier without specifier is invalid

    // Test invalid multiplier formats
    assert(cstruct_sizeof("-1b") == 0);         // Negative multiplier is invalid
    assert(cstruct_sizeof("b3") == 0);          // Multiplier after specifier is invalid
    assert(cstruct_sizeof("3b2") == 0);         // Trailing number without specifier is invalid
    assert(cstruct_sizeof("2147483648b") == 0); // Multiplier too large (overflow)

    // String specifier tests
    // Note: The exact behavior for string handling wasn't specified,
    // so these are assumptions that might need adjustment

    // Test for special case where 's' might be a special case
    // Depending on implementation, these might need to be revised
    assert(cstruct_sizeof("s") > 0);  // String should have some size
    assert(cstruct_sizeof("2s") > 0); // 2 strings

    // Additional validation tests
    assert(cstruct_sizeof("bhi?") == 0); // Invalid due to '?'
    assert(cstruct_sizeof("3z") == 0);   // Invalid specifier with multiplier
    assert(cstruct_sizeof("3b2z") == 0); // Valid followed by invalid

    printf("All tests passed!\n");
}

int main(int argc, char **argv)
{
    test_cstruct_sizeof();
    return 0;
}
