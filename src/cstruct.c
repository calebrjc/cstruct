#include "cstruct.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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
///         multiplier, or -1 if the character is not a valid type.
static ssize_t __cstruct_calculate_size(char c, int multiplier);

// Packing/Unpacking Functions ---------------------------------------------------------------------

typedef uint16_t (*__cstruct_pack16_f)(uint16_t x);
typedef uint16_t (*__cstruct_unpack16_f)(uint16_t x);
typedef uint32_t (*__cstruct_pack32_f)(uint32_t x);
typedef uint32_t (*__cstruct_unpack32_f)(uint32_t x);
typedef uint64_t (*__cstruct_pack64_f)(uint64_t x);
typedef uint64_t (*__cstruct_unpack64_f)(uint64_t x);
typedef uint32_t (*__cstruct_pack_float_f)(float x);
typedef float (*__cstruct_unpack_float_f)(uint32_t x);
typedef uint32_t (*__cstruct_pack_double_f)(double x);
typedef double (*__cstruct_unpack_double_f)(uint32_t x);

static inline uint16_t __cstruct_pack_be16(uint16_t x);
static inline uint16_t __cstruct_pack_le16(uint16_t x);
static inline uint16_t __cstruct_unpack_be16(uint16_t x);
static inline uint16_t __cstruct_unpack_le16(uint16_t x);

static inline uint32_t __cstruct_pack_be32(uint32_t x);
static inline uint32_t __cstruct_pack_le32(uint32_t x);
static inline uint32_t __cstruct_unpack_be32(uint32_t x);
static inline uint32_t __cstruct_unpack_le32(uint32_t x);

static inline uint64_t __cstruct_pack_be64(uint64_t x);
static inline uint64_t __cstruct_pack_le64(uint64_t x);
static inline uint64_t __cstruct_unpack_be64(uint64_t x);
static inline uint64_t __cstruct_unpack_le64(uint64_t x);

static inline uint32_t __cstruct_pack_float_be(float x);
static inline uint32_t __cstruct_pack_float_le(float x);
static inline float    __cstruct_unpack_float_be(uint32_t x);
static inline float    __cstruct_unpack_float_le(uint32_t x);

static inline uint32_t __cstruct_pack_double_be(double x);
static inline uint32_t __cstruct_pack_double_le(double x);
static inline double   __cstruct_unpack_double_be(uint32_t x);
static inline double   __cstruct_unpack_double_le(uint32_t x);

// Public API --------------------------------------------------------------------------------------

ssize_t cstruct_pack(const char *format, void *buffer, size_t buffer_size, ...)
{
    if (!format || *format == '\0')
    {
        return -1;
    }

    size_t  i          = 0;
    ssize_t total_size = 0;
    va_list args;

    // NOTE(Caleb):
    // - Select packing functions here to avoid unnecessary branching in the packing loop
    // - Assume big endian unless otherwise specified
    __cstruct_pack16_f      pack16      = __cstruct_pack_be16;
    __cstruct_pack32_f      pack32      = __cstruct_pack_be32;
    __cstruct_pack64_f      pack64      = __cstruct_pack_be64;
    __cstruct_pack_float_f  pack_float  = __cstruct_pack_float_be;
    __cstruct_pack_double_f pack_double = __cstruct_pack_double_be;

    if (format[0] == '!' || format[0] == '<' || format[0] == '>')
    {
        if (format[0] == '<')
        {
            pack16      = __cstruct_pack_le16;
            pack32      = __cstruct_pack_le32;
            pack64      = __cstruct_pack_le64;
            pack_float  = __cstruct_pack_float_le;
            pack_double = __cstruct_pack_double_le;
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
                        x          = pack16(x);

                        memcpy(dest + j, &x, 2);

                        break;
                    }

                    case 'i':
                    case 'I':
                    case 'l':
                    case 'L':
                    {
                        uint32_t x = (uint32_t)va_arg(args, int);
                        x          = pack32(x);

                        memcpy(dest + j, &x, 4);

                        break;
                    }

                    case 'q':
                    case 'Q':
                    {
                        uint64_t x = (uint64_t)va_arg(args, uint64_t);
                        x          = pack64(x);

                        memcpy(dest + j, &x, 8);

                        break;
                    }

                    case 'f':
                    {
                        float    f = (float)va_arg(args, double);
                        uint32_t u = pack_float(f);

                        memcpy(dest + j, &u, 4);

                        break;
                    }

                    case 'd':
                    {
                        double   d = (double)va_arg(args, double);
                        uint64_t u = pack_double(d);

                        memcpy(dest + j, &u, 8);

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
        return -1;
    }

    size_t  i          = 0;
    ssize_t bytes_read = 0;
    va_list args;

    // NOTE(Caleb):
    // - Select unpacking functions here to avoid unnecessary branching in the unpacking loop
    // - Assume big endian unless otherwise specified
    __cstruct_unpack16_f      unpack16      = __cstruct_unpack_be16;
    __cstruct_unpack32_f      unpack32      = __cstruct_unpack_be32;
    __cstruct_unpack64_f      unpack64      = __cstruct_unpack_be64;
    __cstruct_unpack_float_f  unpack_float  = __cstruct_unpack_float_be;
    __cstruct_unpack_double_f unpack_double = __cstruct_unpack_double_be;

    if (format[0] == '!' || format[0] == '<' || format[0] == '>')
    {
        if (format[0] == '<')
        {
            unpack16      = __cstruct_unpack_le16;
            unpack32      = __cstruct_unpack_le32;
            unpack64      = __cstruct_unpack_le64;
            unpack_float  = __cstruct_unpack_float_le;
            unpack_double = __cstruct_unpack_double_le;
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

        ssize_t size = __cstruct_calculate_size(format_char, multiplier);
        if (size <= 0)
        {
            va_end(args);
            return -1;
        }

        // NOTE(Caleb): Ensure that we don't read past the end of the buffer
        if ((size_t)(bytes_read + size) > buffer_size)
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

        uint8_t *src = (uint8_t *)buffer + bytes_read;

        if (format_char == 's')
        {
            void *dest = va_arg(args, void *);
            memcpy(dest, src, size);
        }
        else
        {
            for (ssize_t j = 0; j < size; j += size / multiplier)
            {
                switch (format_char)
                {
                    // ...
                    case 'b':
                    case 'B':
                    {
                        uint8_t x = *(src + j);

                        uint8_t *dest = va_arg(args, uint8_t *);
                        *dest         = x;

                        break;
                    }

                    case 'h':
                    case 'H':
                    {
                        uint16_t x = 0;
                        memcpy(&x, src + j, 2);

                        uint16_t *dest = va_arg(args, uint16_t *);
                        *dest          = unpack16(x);

                        break;
                    }

                    case 'i':
                    case 'I':
                    case 'l':
                    case 'L':
                    {
                        uint32_t x = 0;
                        memcpy(&x, src + j, 4);

                        uint32_t *dest = va_arg(args, uint32_t *);
                        *dest          = unpack32(x);

                        break;
                    }

                    case 'q':
                    case 'Q':
                    {
                        uint64_t x = 0;
                        memcpy(&x, src + j, 8);

                        uint64_t *dest = va_arg(args, uint64_t *);
                        *dest          = unpack64(x);

                        break;
                    }

                    case 'f':
                    {
                        uint32_t x = 0;
                        memcpy(&x, src + j, 4);

                        float *dest = va_arg(args, float *);
                        *dest       = unpack_float(x);

                        break;
                    }

                    case 'd':
                    {
                        uint64_t x = 0;
                        memcpy(&x, src + j, 8);

                        double *dest = va_arg(args, double *);
                        *dest        = unpack_double(x);

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

ssize_t cstruct_sizeof(const char *format)
{
    if (!format || *format == '\0')
    {
        return -1;
    }

    size_t  i          = 0;
    ssize_t total_size = 0;

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
            return -1;
        }

        // NOTE(Caleb): At this point, format[i] is the next format character
        ssize_t size = __cstruct_calculate_size(format[i], multiplier);
        if (size <= 0)
        {
            return -1;
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

static ssize_t __cstruct_calculate_size(char c, int multiplier)
{
    ssize_t size = 0;

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
            return -1;
    }

    return size * multiplier;
}

static inline uint16_t __cstruct_pack_be16(uint16_t x)
{
    uint8_t data[2] = {(uint8_t)(x >> 8), (uint8_t)(x & 0xFF)};

    uint16_t result = 0;
    memcpy(&result, data, 2);

    return result;
}

static inline uint16_t __cstruct_pack_le16(uint16_t x)
{
    uint8_t data[2] = {(uint8_t)(x & 0xFF), (uint8_t)(x >> 8)};

    uint16_t result = 0;
    memcpy(&result, data, 2);

    return result;
}

static inline uint16_t __cstruct_unpack_be16(uint16_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint16_t)data[0] << 8) | data[1];
}

static inline uint16_t __cstruct_unpack_le16(uint16_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint16_t)data[1] << 8) | data[0];
}

static inline uint32_t __cstruct_pack_be32(uint32_t x)
{
    uint8_t data[4] = {
        (uint8_t)(x >> 24),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)(x & 0xFF),
    };

    uint32_t result = 0;
    memcpy(&result, data, 4);

    return result;
}

static inline uint32_t __cstruct_pack_le32(uint32_t x)
{
    uint8_t data[4] = {
        (uint8_t)(x & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)(x >> 24),
    };

    uint32_t result = 0;
    memcpy(&result, data, 4);

    return result;
}

static inline uint32_t __cstruct_unpack_be32(uint32_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8)
         | data[3];
}

static inline uint32_t __cstruct_unpack_le32(uint32_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8)
         | data[0];
}

static inline uint64_t __cstruct_pack_be64(uint64_t x)
{
    uint8_t data[8] = {
        (uint8_t)(x >> 56),
        (uint8_t)((x >> 48) & 0xFF),
        (uint8_t)((x >> 40) & 0xFF),
        (uint8_t)((x >> 32) & 0xFF),
        (uint8_t)((x >> 24) & 0xFF),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)(x & 0xFF),
    };

    uint64_t result = 0;
    memcpy(&result, data, 8);

    return result;
}

static inline uint64_t __cstruct_pack_le64(uint64_t x)
{
    uint8_t data[8] = {
        (uint8_t)(x & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)((x >> 24) & 0xFF),
        (uint8_t)((x >> 32) & 0xFF),
        (uint8_t)((x >> 40) & 0xFF),
        (uint8_t)((x >> 48) & 0xFF),
        (uint8_t)(x >> 56),
    };

    uint64_t result = 0;
    memcpy(&result, data, 8);

    return result;
}

static inline uint64_t __cstruct_unpack_be64(uint64_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) | ((uint64_t)data[2] << 40)
         | ((uint64_t)data[3] << 32) | ((uint64_t)data[4] << 24) | ((uint64_t)data[5] << 16)
         | ((uint64_t)data[6] << 8) | data[7];
}

static inline uint64_t __cstruct_unpack_le64(uint64_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint64_t)data[7] << 56) | ((uint64_t)data[6] << 48) | ((uint64_t)data[5] << 40)
         | ((uint64_t)data[4] << 32) | ((uint64_t)data[3] << 24) | ((uint64_t)data[2] << 16)
         | ((uint64_t)data[1] << 8) | data[0];
}

static inline uint32_t __cstruct_pack_float_be(float x)
{
    uint32_t y = 0;
    memcpy(&y, &x, sizeof(float));

    return __cstruct_pack_be32(y);
}

static inline uint32_t __cstruct_pack_float_le(float x)
{
    uint32_t y = 0;
    memcpy(&y, &x, sizeof(float));

    return __cstruct_pack_le32(y);
}

static inline float __cstruct_unpack_float_be(uint32_t x)
{
    float    y = 0;
    uint32_t z = __cstruct_unpack_be32(x);
    memcpy(&y, &z, sizeof(float));

    return y;
}

static inline float __cstruct_unpack_float_le(uint32_t x)
{
    float    y = 0;
    uint32_t z = __cstruct_unpack_le32(x);
    memcpy(&y, &z, sizeof(float));

    return y;
}

static inline uint32_t __cstruct_pack_double_be(double x)
{
    uint64_t y = 0;
    memcpy(&y, &x, sizeof(double));

    return __cstruct_pack_be64(y);
}

static inline uint32_t __cstruct_pack_double_le(double x)
{
    uint64_t y = 0;
    memcpy(&y, &x, sizeof(double));

    return __cstruct_pack_le64(y);
}

static inline double __cstruct_unpack_double_be(uint32_t x)
{
    double   y = 0;
    uint64_t z = __cstruct_unpack_be64(x);
    memcpy(&y, &z, sizeof(double));

    return y;
}

static inline double __cstruct_unpack_double_le(uint32_t x)
{
    double   y = 0;
    uint64_t z = __cstruct_unpack_le64(x);
    memcpy(&y, &z, sizeof(double));

    return y;
}
