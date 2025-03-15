#include "minunit.h"

#include "cstruct.h"

MU_TEST(test_sizeof_basic_specifiers)
{
    mu_check(cstruct_sizeof("x") == 1);
    mu_check(cstruct_sizeof("b") == 1);
    mu_check(cstruct_sizeof("B") == 1);
    mu_check(cstruct_sizeof("h") == 2);
    mu_check(cstruct_sizeof("H") == 2);
    mu_check(cstruct_sizeof("i") == 4);
    mu_check(cstruct_sizeof("I") == 4);
    mu_check(cstruct_sizeof("l") == 4);
    mu_check(cstruct_sizeof("L") == 4);
    mu_check(cstruct_sizeof("q") == 8);
    mu_check(cstruct_sizeof("Q") == 8);
    mu_check(cstruct_sizeof("f") == 4);
    mu_check(cstruct_sizeof("d") == 8);
}

MU_TEST(test_sizeof_empty_and_invalid)
{
    mu_check(cstruct_sizeof("") == 0);
    mu_check(cstruct_sizeof(NULL) == 0);

    // Invalid specifiers
    mu_check(cstruct_sizeof("z") == 0);
    mu_check(cstruct_sizeof("A") == 0);
}

MU_TEST(test_sizeof_combinations)
{
    // 1 + 1 = 2
    mu_check(cstruct_sizeof("bB") == 2);

    // 2 + 2 = 4
    mu_check(cstruct_sizeof("hH") == 4);

    // 4 + 4 = 8
    mu_check(cstruct_sizeof("iI") == 8);

    // 4 + 4 = 8
    mu_check(cstruct_sizeof("lL") == 8);

    // 8 + 8 = 16
    mu_check(cstruct_sizeof("qQ") == 16);

    // 4 + 8 = 12
    mu_check(cstruct_sizeof("fd") == 12);

    // 1 + 2 + 4 + 8 + 8 = 23
    mu_check(cstruct_sizeof("bHiQd") == 23);

    // 1 + 1 + 1 + 1 + 1 + 1 = 6
    mu_check(cstruct_sizeof("xxbbBB") == 6);
}

MU_TEST(test_sizeof_multipliers)
{
    // 3 × int8_t = 3
    mu_check(cstruct_sizeof("3b") == 3);

    // 2 × uint16_t = 4
    mu_check(cstruct_sizeof("2H") == 4);

    // 4 × int32_t = 16
    mu_check(cstruct_sizeof("4i") == 16);

    // 10 × nul byte = 10
    mu_check(cstruct_sizeof("10x") == 10);

    // 3 × double = 24
    mu_check(cstruct_sizeof("3d") == 24);
}

MU_TEST(test_sizeof_combined_multipliers)
{
    // (2 × int8_t) + (3 × uint8_t) = 5
    mu_check(cstruct_sizeof("2b3B") == 5);

    // (3 × int16_t) + (2 × int32_t) = 14
    mu_check(cstruct_sizeof("3h2i") == 14);

    // (2 × nul) + (3 × int8_t) + (4 × uint16_t) + (2 × int32_t) = 21
    mu_check(cstruct_sizeof("2x3b4H2i") == 21);
}

MU_TEST(test_sizeof_multi_digit_multipliers)
{
    // 12 × int8_t = 12
    mu_check(cstruct_sizeof("12b") == 12);

    // 25 × nul byte = 25
    mu_check(cstruct_sizeof("25x") == 25);

    // 100 × int8_t = 100
    mu_check(cstruct_sizeof("100b") == 100);
}

MU_TEST(test_sizeof_multiplier_edge_cases)
{
    // 0 × anything = 0
    mu_check(cstruct_sizeof("0b") == 0);

    // Leading zero in multiplier should work
    mu_check(cstruct_sizeof("01h") == 2);

    // Multiplier without specifier is invalid
    mu_check(cstruct_sizeof("1") == 0);

    // Multiplier without specifier is invalid
    mu_check(cstruct_sizeof("42") == 0);
}

MU_TEST(test_sizeof_invalid_multipliers)
{
    // Negative multiplier is invalid
    mu_check(cstruct_sizeof("-1b") == 0);

    // Multiplier after specifier is invalid
    mu_check(cstruct_sizeof("b3") == 0);

    // Trailing number without specifier is invalid
    mu_check(cstruct_sizeof("3b2") == 0);

    // Multiplier too large (overflow)
    mu_check(cstruct_sizeof("2147483648b") == 0);
}

MU_TEST(test_sizeof_string_specifier)
{
    // String should have some size
    mu_check(cstruct_sizeof("s") > 0);

    // 2 strings
    mu_check(cstruct_sizeof("2s") > 0);
}

MU_TEST(test_sizeof_additional_validation)
{
    // Invalid due to '?'
    mu_check(cstruct_sizeof("bhi?") == 0);

    // Invalid specifier with multiplier
    mu_check(cstruct_sizeof("3z") == 0);

    // Valid followed by invalid
    mu_check(cstruct_sizeof("3b2z") == 0);
}

MU_TEST(test_sizeof_additional_corner_cases)
{
    // Interspersed multipliers and characters
    mu_check(cstruct_sizeof("2b3i2b") == 16); // 2*1 + 3*4 + 2*1 = 16

    // Maximum valid multiplier (if implementation supports it)
    mu_check(cstruct_sizeof("2147483647x") > 0);

    // Format with whitespace (behavior depends on implementation)
    mu_check(cstruct_sizeof("2b 3i") == 0); // Expected to fail with current implementation

    // Format ending with digit
    mu_check(cstruct_sizeof("2b3") == 0); // Should fail - trailing digit
}

MU_TEST_SUITE(test_suite)
{
    MU_RUN_TEST(test_sizeof_basic_specifiers);
    MU_RUN_TEST(test_sizeof_empty_and_invalid);
    MU_RUN_TEST(test_sizeof_combinations);
    MU_RUN_TEST(test_sizeof_multipliers);
    MU_RUN_TEST(test_sizeof_combined_multipliers);
    MU_RUN_TEST(test_sizeof_multi_digit_multipliers);
    MU_RUN_TEST(test_sizeof_multiplier_edge_cases);
    MU_RUN_TEST(test_sizeof_invalid_multipliers);
    MU_RUN_TEST(test_sizeof_string_specifier);
    MU_RUN_TEST(test_sizeof_additional_validation);
    MU_RUN_TEST(test_sizeof_additional_corner_cases);
}

int main(void)
{
    MU_RUN_SUITE(test_suite);
    MU_REPORT();

    return MU_EXIT_CODE;
}
