#include "cstruct.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

/// Return true if the character is a digit, and false otherwise.
/// @param[in] c Character to check.
/// @return True if the character is a digit, and false otherwise.
static inline bool __cstruct_isdigit(char c);

/// @brief Return the next multiplier for the given format string, and advance the index to that of
///        the next format character.
/// @param format The format string.
/// @param[inout] i On entry, the supposed start index of the multiplier.
///                 On exit, the index of the supposed next format character.
/// @return The multiplier, or -1 if the format string is invalid.
static int32_t __cstruct_parse_multiplier(const char *format, size_t *i);

/// Return the size of the type which the given character represents.
/// @param[in] c Character to check.
/// @param[in] multiplier Multiplier to apply to the size.
/// @return The size of the type which the given character represents multiplied by the given
///         multiplier, or 0 if the character is not a valid type.
static size_t __cstruct_calculate_size(char c, int multiplier);

// Public API --------------------------------------------------------------------------------------

size_t cstruct_sizeof(const char *format)
{
    if (!format || *format == '\0')
    {
        return 0;
    }

    size_t i = 0, total_size = 0;
    if (format[0] == '!' || format[0] == '<' || format[0] == '>')
    {
        i++;
    }

    while (format[i] != '\0')
    {
        int32_t multiplier = __cstruct_parse_multiplier(format, &i);
        if (multiplier <= 0)
        {
            return 0;
        }

        // NOTE(Caleb): At this point, format[i] is the next format character.
        size_t size = __cstruct_calculate_size(format[i], multiplier);
        if (size == 0)
        {
            return 0;
        }

        total_size += size;
        i++;
    }

    return total_size;
}

// Private Helpers ---------------------------------------------------------------------------------

static inline bool __cstruct_isdigit(char c)
{
    return '0' <= c && c <= '9';
}

static int32_t __cstruct_parse_multiplier(const char *format, size_t *i)
{
    // NOTE(Caleb):
    // - Assume that format is not NULL and is not empty, as the calling function should have
    //   already validated this.
    // - Assume that i is not NULL, as the calling function should have already validated this.

    if (!__cstruct_isdigit(format[*i]))
    {
        return 1;
    }

    int32_t multiplier = 0;
    do
    {
        multiplier = (multiplier * 10) + (format[*i] - '0');
        (*i)++;
    } while (__cstruct_isdigit(format[*i]));

    // NOTE(Caleb):
    // If the multiplier somehow gets overflowed, just return 0 (invalid format).
    // I don't intend to support structures with sizes larger than a positive int32_t.
    if (multiplier <= 0)
    {
        return -1;
    }

    return multiplier;
}

static size_t __cstruct_calculate_size(char c, int multiplier)
{
    size_t base_size = 0;

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
