#include "jcsw/binpack.h"

#include <stdbool.h>

/*
 * TODO(Caleb):
 * - Allow little-endian and native modes
 */

/// Return true if the character is a digit, and false otherwise.
/// @param [in] c Character to check.
/// @return True if the character is a digit, and false otherwise.
static inline bool __binpack_isdigit(char c);

/// Return the size of the type which the given character represents.
/// @param [in] c Character to check.
/// @param [in] multiplier Multiplier to apply to the size.
/// @return The size of the type which the given character represents multiplied by the given
///         multiplier, or 0 if the character is not a valid type.
static size_t __binpack_calculate_size(char c, int multiplier);

size_t binpack_sizeof(const char *format)
{
    if (!format)
    {
        return 0;
    }

    size_t i = 0, total_size = 0;
    while (format[i] != '\0')
    {
        bool    multiplier_set = false;
        int32_t multiplier     = 1;

        while (__binpack_isdigit(format[i]))
        {
            if (!multiplier_set)
            {
                multiplier_set = true;
                multiplier     = 0;
            }

            multiplier *= 10;
            multiplier += format[i] - '0';
            i++;
        }

        // NOTE(Caleb): If the multiplier somehow gets overflowed, just return 0. I don't intend to
        //              support structures with sizes so large that an int32_t can't hold it.
        if (multiplier < 0)
        {
            return 0;
        }

        size_t size = __binpack_calculate_size(format[i], multiplier);
        if (size == 0)
        {
            return 0;
        }

        total_size += size;
        i++;
    }

    return total_size;
}

static inline bool __binpack_isdigit(char c)
{
    return '0' <= c && c <= '9';
}

static size_t __binpack_calculate_size(char c, int multiplier)
{
    size_t base_size = 0;

    // TODO(Caleb): Implement 'p'
    switch (c)
    {
        case 'x':
        case 'b':
        case 'B':
        case 's':
            base_size = 1;
            break;

        case 'h':
        case 'H':
            base_size = 2;
            break;

        case 'i':
        case 'I':
        case 'l':
        case 'L':
        case 'f':
            base_size = 4;
            break;

        case 'q':
        case 'Q':
        case 'd':
            base_size = 8;
            break;

        default:
            return 0;
    }

    return base_size * multiplier;
}
