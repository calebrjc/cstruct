#include "cstruct.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// clang-format off
#define __CSTRUCT_IS_ON_BE_ARCH                                                                    \
    ((defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN)                                       \
     || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)                        \
     || defined(__BIG_ENDIAN__)                                                                    \
     || defined(__ARMEB__)                                                                         \
     || defined(__THUMBEB__)                                                                       \
     || defined(__AARCH64EB__)                                                                     \
     || defined(_MIPSEB)                                                                           \
     || defined(__MIPSEB)                                                                          \
     || defined(__MIPSEB__))

#define __CSTRUCT_IS_ON_LE_ARCH                                                                    \
    ((defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN)                                    \
     || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)                     \
     || defined(__LITTLE_ENDIAN__)                                                                 \
     || defined(__ARMEL__)                                                                         \
     || defined(__THUMBEL__)                                                                       \
     || defined(__AARCH64EL__)                                                                     \
     || defined(_MIPSEL)                                                                           \
     || defined(__MIPSEL)                                                                          \
     || defined(__MIPSEL__))
// clang-format on

#define __CSTRUCT_SYS_BYTE_ORDER_BE 4321
#define __CSTRUCT_SYS_BYTE_ORDER_LE 1234

#if (__CSTRUCT_IS_ON_BE_ARCH)
#define __CSTRUCT_SYS_BYTE_ORDER __CSTRUCT_SYS_BYTE_ORDER_BE
#elif (__CSTRUCT_IS_ON_LE_ARCH)
#define __CSTRUCT_SYS_BYTE_ORDER __CSTRUCT_SYS_BYTE_ORDER_LE
#else
#error                                                                                             \
    "error: Unable to detect system byte order. Please define __CSTRUCT_SYS_BYTE_ORDER as __CSTRUCT_SYS_BYTE_ORDER_BE or __CSTRUCT_SYS_BYTE_ORDER_LE."
#endif

#if (__CSTRUCT_SYS_BYTE_ORDER == __CSTRUCT_SYS_BYTE_ORDER_BE)
#define __cstruct_htobe16(__x) (__x)
#define __cstruct_htobe32(__x) (__x)
#define __cstruct_htobe64(__x) (__x)
#define __cstruct_htole16(__x) (__cstruct_bswap16(__x))
#define __cstruct_htole32(__x) (__cstruct_bswap32(__x))
#define __cstruct_htole64(__x) (__cstruct_bswap64(__x))
#define __cstruct_betoh16(__x) (__x)
#define __cstruct_betoh32(__x) (__x)
#define __cstruct_betoh64(__x) (__x)
#define __cstruct_letoh16(__x) (__cstruct_bswap16(__x))
#define __cstruct_letoh32(__x) (__cstruct_bswap32(__x))
#define __cstruct_letoh64(__x) (__cstruct_bswap64(__x))
#else // __CSTRUCT_SYS_BYTE_ORDER == __CSTRUCT_SYS_BYTE_ORDER_LE)
#define __cstruct_htobe16(__x) (__cstruct_bswap16(__x))
#define __cstruct_htobe32(__x) (__cstruct_bswap32(__x))
#define __cstruct_htobe64(__x) (__cstruct_bswap64(__x))
#define __cstruct_htole16(__x) (__x)
#define __cstruct_htole32(__x) (__x)
#define __cstruct_htole64(__x) (__x)
#define __cstruct_betoh16(__x) (__cstruct_bswap16(__x))
#define __cstruct_betoh32(__x) (__cstruct_bswap32(__x))
#define __cstruct_betoh64(__x) (__cstruct_bswap64(__x))
#define __cstruct_letoh16(__x) (__x)
#define __cstruct_letoh32(__x) (__x)
#define __cstruct_letoh64(__x) (__x)
#endif

typedef enum
{
    __CSTRUCT_BYTE_ORDER_BE = 0,
    __CSTRUCT_BYTE_ORDER_LE = 1,
} __cstruct_byte_order_e;

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

static inline uint16_t __cstruct_bswap16(uint16_t x);
static inline uint32_t __cstruct_bswap32(uint32_t x);
static inline uint64_t __cstruct_bswap64(uint64_t x);

// Public API --------------------------------------------------------------------------------------

ssize_t cstruct_pack(const char *format, void *buffer, size_t buffer_size, ...)
{
    if (!format || *format == '\0')
    {
        return -1;
    }

    __cstruct_byte_order_e byte_order = __CSTRUCT_BYTE_ORDER_BE;
    size_t                 i          = 0;
    size_t                 total_size = 0;
    va_list                args;

    // NOTE(Caleb): Assume big endian unless otherwise specified
    if (format[0] == '!' || format[0] == '<' || format[0] == '>')
    {
        if (format[0] == '<')
        {
            byte_order = __CSTRUCT_BYTE_ORDER_LE;
        }

        i++;
    }

    va_start(args, buffer_size);

    while (format[i] != '\0')
    {
        int32_t multiplier = __cstruct_parse_multiplier(format, &i);
        if (multiplier <= 0)
        {
            va_end(args);
            return -1;
        }

        char format_char = format[i];

        size_t size = __cstruct_calculate_size(format_char, multiplier);
        if (size == 0)
        {
            va_end(args);
            return -1;
        }

        // NOTE(Caleb): If true, the buffer is too small to fit the next value
        if (total_size + size > buffer_size)
        {
            va_end(args);
            return -1;
        }

        uint8_t *dest = (uint8_t *)buffer + total_size;
        if (format_char == 'x')
        {
            memset(dest, 0, size);
        }
        else if (format_char == 's')
        {
            void *src = va_arg(args, void *);
            memset(dest, 0, size);

            if (src)
            {
                memcpy(dest, src, size);
            }
        }
        else
        {
            for (size_t j = 0; j < size; j += size / multiplier)
            {
                switch (format_char)
                {
                    case 'b':
                    case 'B':
                    {
                        uint8_t x = (uint8_t)va_arg(args, int);
                        memcpy(dest + j, &x, 1);

                        break;
                    }

                    case 'h':
                    case 'H':
                    {
                        uint16_t x = (uint16_t)va_arg(args, int);
                        x          = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_htole16(x)
                                                                             : __cstruct_htobe16(x);

                        memcpy(dest + j, &x, 2);

                        break;
                    }

                    case 'i':
                    case 'I':
                    case 'l':
                    case 'L':
                    {
                        uint32_t x = (uint32_t)va_arg(args, int);
                        x          = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_htole32(x)
                                                                             : __cstruct_htobe32(x);

                        memcpy(dest + j, &x, 4);

                        break;
                    }

                    case 'q':
                    case 'Q':
                    {
                        uint64_t x = (uint64_t)va_arg(args, uint64_t);
                        x          = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_htole64(x)
                                                                             : __cstruct_htobe64(x);

                        memcpy(dest + j, &x, 8);

                        break;
                    }

                    case 'f':
                    {
                        union
                        {
                            float    f;
                            uint32_t i;
                        } u;

                        u.f = (float)va_arg(args, double);
                        u.i = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_htole32(u.i)
                                                                      : __cstruct_htobe32(u.i);

                        memcpy(dest + j, &u.i, 4);

                        break;
                    }

                    case 'd':
                    {
                        union
                        {
                            double   d;
                            uint64_t i;
                        } u;

                        u.d = (double)va_arg(args, double);
                        u.i = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_htole64(u.i)
                                                                      : __cstruct_htobe64(u.i);

                        memcpy(dest + j, &u.i, 8);

                        break;
                    }

                    default:
                        va_end(args);
                        return -1;
                }
            }
        }

        total_size += size;
        i++;
    }

    va_end(args);
    return total_size;
}

ssize_t cstruct_unpack(const char *format, const void *buffer, size_t buffer_size, ...)
{
    if (!format || *format == '\0')
    {
        return 0;
    }

    __cstruct_byte_order_e byte_order = __CSTRUCT_BYTE_ORDER_BE;
    size_t                 i          = 0;
    size_t                 bytes_read = 0;
    va_list                args;

    // NOTE(Caleb): Assume big endian unless otherwise specified
    if (format[0] == '!' || format[0] == '<' || format[0] == '>')
    {
        if (format[0] == '<')
        {
            byte_order = __CSTRUCT_BYTE_ORDER_LE;
        }

        i++;
    }

    va_start(args, buffer_size);

    while (format[i] != '\0')
    {
        int32_t multiplier = __cstruct_parse_multiplier(format, &i);
        if (multiplier <= 0)
        {
            va_end(args);
            return -1;
        }

        char format_char = format[i];

        size_t size = __cstruct_calculate_size(format_char, multiplier);
        if (size == 0)
        {
            return 0;
        }

        // NOTE(Caleb): Ensure that we don't read past the end of the buffer
        if (bytes_read + size > buffer_size)
        {
            va_end(args);
            return -1;
        }

        if (format_char == 'x')
        {
            bytes_read += size;
            i++;

            continue;
        }

        if (format_char == 's')
        {
            void *dest = va_arg(args, void *);
            memcpy(dest, buffer + bytes_read, size);
        }
        else
        {
            for (size_t j = 0; j < size; j += size / multiplier)
            {
                switch (format_char)
                {
                    // ...
                    case 'b':
                    case 'B':
                    {
                        uint8_t x = *((uint8_t *)(buffer + bytes_read + j));

                        uint8_t *dest = va_arg(args, uint8_t *);
                        *dest         = x;

                        break;
                    }

                    case 'h':
                    case 'H':
                    {
                        uint16_t x = *((uint16_t *)(buffer + bytes_read + j));
                        x          = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_letoh16(x)
                                                                             : __cstruct_betoh16(x);

                        uint16_t *dest = va_arg(args, uint16_t *);
                        *dest          = x;

                        break;
                    }

                    case 'i':
                    case 'I':
                    case 'l':
                    case 'L':
                    {
                        uint32_t x = *((uint32_t *)(buffer + bytes_read + j));
                        x          = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_letoh32(x)
                                                                             : __cstruct_betoh32(x);

                        uint32_t *dest = va_arg(args, uint32_t *);
                        *dest          = x;

                        break;
                    }

                    case 'q':
                    case 'Q':
                    {
                        uint64_t x = *((uint64_t *)(buffer + bytes_read + j));
                        x          = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_letoh64(x)
                                                                             : __cstruct_betoh64(x);

                        uint64_t *dest = va_arg(args, uint64_t *);
                        *dest          = x;

                        break;
                    }

                    case 'f':
                    {
                        union
                        {
                            float    f;
                            uint32_t i;
                        } u;

                        u.f = *((float *)(buffer + bytes_read + j));
                        u.i = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_letoh32(u.i)
                                                                      : __cstruct_betoh32(u.i);

                        float *dest = va_arg(args, float *);
                        *dest       = u.f;

                        break;
                    }

                    case 'd':
                    {
                        union
                        {
                            double   d;
                            uint64_t i;
                        } u;

                        u.d = *((double *)(buffer + bytes_read + j));
                        u.i = (byte_order == __CSTRUCT_BYTE_ORDER_LE) ? __cstruct_letoh64(u.i)
                                                                      : __cstruct_betoh64(u.i);

                        double *dest = va_arg(args, double *);
                        *dest        = u.d;

                        break;
                    }

                    default:
                        va_end(args);
                        return -1;
                }
            }
        }

        bytes_read += size;
        i++;
    }

    return bytes_read;
}

size_t cstruct_sizeof(const char *format)
{
    if (!format || *format == '\0')
    {
        return 0;
    }

    size_t i          = 0;
    size_t total_size = 0;

    // NOTE(Caleb): Skip over the byte order specifier
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

        // NOTE(Caleb): At this point, format[i] is the next format character
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
    //   already validated this
    // - Assume that i is not NULL, as the calling function should have already validated this

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
    // If the multiplier somehow gets overflowed, just return 0 (invalid format)
    // I don't intend to support structures with sizes larger than a positive int32_t
    if (multiplier <= 0)
    {
        return -1;
    }

    return multiplier;
}

static size_t __cstruct_calculate_size(char c, int multiplier)
{
    size_t size = 0;

    switch (c)
    {
        case 'x':
        case 'b':
        case 'B':
        case 's':
            size = 1;
            break;

        case 'h':
        case 'H':
            size = 2;
            break;

        case 'i':
        case 'I':
        case 'l':
        case 'L':
        case 'f':
            size = 4;
            break;

        case 'q':
        case 'Q':
        case 'd':
            size = 8;
            break;

        default:
            return 0;
    }

    return size * multiplier;
}

static inline uint16_t __cstruct_bswap16(uint16_t x)
{
    return (uint16_t)(((x >> 8) & 0xFF) | ((x & 0xFF) << 8));
}

static inline uint32_t __cstruct_bswap32(uint32_t x)
{
    return ((x & 0xFF000000U) >> 24) | ((x & 0x00FF0000U) >> 8) | ((x & 0x0000FF00U) << 8)
         | ((x & 0x000000FFU) << 24);
}

static inline uint64_t __cstruct_bswap64(uint64_t x)
{
    return ((x & 0xFF00000000000000ULL) >> 56) | ((x & 0x00FF000000000000ULL) >> 40)
         | ((x & 0x0000FF0000000000ULL) >> 24) | ((x & 0x000000FF00000000ULL) >> 8)
         | ((x & 0x00000000FF000000ULL) << 8) | ((x & 0x0000000000FF0000ULL) << 24)
         | ((x & 0x000000000000FF00ULL) << 40) | ((x & 0x00000000000000FFULL) << 56);
}
