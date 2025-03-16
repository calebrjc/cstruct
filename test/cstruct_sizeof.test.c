#include "minunit.h"

#include "cstruct.h"

MU_TEST(test_basic_format_chars)
{
    mu_assert_int_eq(1, cstruct_sizeof("b")); // int8_t
    mu_assert_int_eq(1, cstruct_sizeof("B")); // uint8_t
    mu_assert_int_eq(2, cstruct_sizeof("h")); // int16_t
    mu_assert_int_eq(2, cstruct_sizeof("H")); // uint16_t
    mu_assert_int_eq(4, cstruct_sizeof("i")); // int32_t
    mu_assert_int_eq(4, cstruct_sizeof("I")); // uint32_t
    mu_assert_int_eq(4, cstruct_sizeof("l")); // int32_t
    mu_assert_int_eq(4, cstruct_sizeof("L")); // uint32_t
    mu_assert_int_eq(8, cstruct_sizeof("q")); // int64_t
    mu_assert_int_eq(8, cstruct_sizeof("Q")); // uint64_t
    mu_assert_int_eq(4, cstruct_sizeof("f")); // float
    mu_assert_int_eq(8, cstruct_sizeof("d")); // double
    mu_assert_int_eq(1, cstruct_sizeof("x")); // padding byte
    mu_assert_int_eq(1, cstruct_sizeof("s")); // single char
}

MU_TEST(test_byte_order_specifiers)
{
    mu_assert_int_eq(cstruct_sizeof("h"), cstruct_sizeof("<h"));
    mu_assert_int_eq(cstruct_sizeof("h"), cstruct_sizeof(">h"));
    mu_assert_int_eq(cstruct_sizeof("h"), cstruct_sizeof("!h"));

    mu_assert_int_eq(cstruct_sizeof("i"), cstruct_sizeof("<i"));
    mu_assert_int_eq(cstruct_sizeof("i"), cstruct_sizeof(">i"));
    mu_assert_int_eq(cstruct_sizeof("i"), cstruct_sizeof("!i"));

    mu_assert_int_eq(cstruct_sizeof("q"), cstruct_sizeof("<q"));
    mu_assert_int_eq(cstruct_sizeof("q"), cstruct_sizeof(">q"));
    mu_assert_int_eq(cstruct_sizeof("q"), cstruct_sizeof("!q"));
}

MU_TEST(test_repeat_counts)
{
    mu_assert_int_eq(4, cstruct_sizeof("4b"));
    mu_assert_int_eq(4, cstruct_sizeof("2h"));
    mu_assert_int_eq(12, cstruct_sizeof("3i"));
    mu_assert_int_eq(16, cstruct_sizeof("2q"));
    mu_assert_int_eq(5, cstruct_sizeof("5x"));
    mu_assert_int_eq(20, cstruct_sizeof("5f"));
    mu_assert_int_eq(24, cstruct_sizeof("3d"));
}

MU_TEST(test_combined_formats)
{
    mu_assert_int_eq(7, cstruct_sizeof("bhl"));      // 1 + 2 + 4
    mu_assert_int_eq(16, cstruct_sizeof("ifd"));     // 4 + 4 + 8
    mu_assert_int_eq(6, cstruct_sizeof("bx2h"));     // 1 + 1 + 2*2
    mu_assert_int_eq(13, cstruct_sizeof("b2xif2s")); // 1 + 2 + 4 + 4 + 2*1
}

MU_TEST(test_combined_with_byte_order)
{
    mu_assert_int_eq(7, cstruct_sizeof("<bhl"));
    mu_assert_int_eq(16, cstruct_sizeof(">ifd"));
    mu_assert_int_eq(6, cstruct_sizeof("!bx2h"));
    mu_assert_int_eq(13, cstruct_sizeof("<b2xif2s"));
}

MU_TEST(test_error_cases)
{
    mu_assert_int_eq(0, cstruct_sizeof(""));            // Empty string
    mu_assert_int_eq(0, cstruct_sizeof("z"));           // Invalid format character
    mu_assert_int_eq(0, cstruct_sizeof("0h"));          // Zero multiplier
    mu_assert_int_eq(0, cstruct_sizeof("h<"));          // Byte order not at beginning
    mu_assert_int_eq(0, cstruct_sizeof("<>h"));         // Multiple byte order specifiers
    mu_assert_int_eq(0, cstruct_sizeof("<<h"));         // Repeated byte order specifier
    mu_assert_int_eq(0, cstruct_sizeof("h<i"));         // Byte order in middle
    mu_assert_int_eq(0, cstruct_sizeof("4294967296h")); // Multiplier overflow
    mu_assert_int_eq(0, cstruct_sizeof("@h"));          // Invalid byte order specifier
}

MU_TEST(test_edge_cases)
{
    mu_assert_int_eq(1000, cstruct_sizeof("1000b"));
    mu_assert_int_eq(2000, cstruct_sizeof("1000h"));
    mu_assert_int_eq(4000, cstruct_sizeof("1000i"));
    mu_assert_int_eq(8000, cstruct_sizeof("1000q"));
    mu_assert_int_eq(10000, cstruct_sizeof("10000x"));
}

MU_TEST_SUITE(test_suite)
{
    MU_RUN_TEST(test_basic_format_chars);
    MU_RUN_TEST(test_byte_order_specifiers);
    MU_RUN_TEST(test_repeat_counts);
    MU_RUN_TEST(test_combined_formats);
    MU_RUN_TEST(test_combined_with_byte_order);
    MU_RUN_TEST(test_error_cases);
    MU_RUN_TEST(test_edge_cases);
}

int main(void)
{
    MU_RUN_SUITE(test_suite);
    MU_REPORT();

    return MU_EXIT_CODE;
}
