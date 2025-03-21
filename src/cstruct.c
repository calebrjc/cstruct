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
    // - Selece packing functions here to avoid unnecessary branching in the packing loop
    // - Assume big endian unless otherwise specified
    __cstruct_pack16_f pack16 = __cstruct_pack_be16;
    __cstruct_pack32_f pack32 = __cstruct_pack_be32;
    __cstruct_pack64_f pack64 = __cstruct_pack_be64;

    if (format[0] == '!' || format[0] == '<' || format[0] == '>')
    {
        if (format[0] == '<')
        {
            pack16 = __cstruct_pack_le16;
            pack32 = __cstruct_pack_le32;
            pack64 = __cstruct_pack_le64;
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
                        union
                        {
                            float    f;
                            uint32_t i;
                        } u;

                        u.f = (float)va_arg(args, double);
                        u.i = pack32(u.i);

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
                        u.i = pack64(u.i);

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
        return -1;
    }

    size_t  i          = 0;
    ssize_t bytes_read = 0;
    va_list args;

    // NOTE(Caleb):
    // - Selece unpacking functions here to avoid unnecessary branching in the unpacking loop
    // - Assume big endian unless otherwise specified
    __cstruct_unpack16_f unpack16 = __cstruct_unpack_be16;
    __cstruct_unpack32_f unpack32 = __cstruct_unpack_be32;
    __cstruct_unpack64_f unpack64 = __cstruct_unpack_be64;

    if (format[0] == '!' || format[0] == '<' || format[0] == '>')
    {
        if (format[0] == '<')
        {
            unpack16 = __cstruct_unpack_le16;
            unpack32 = __cstruct_unpack_le32;
            unpack64 = __cstruct_unpack_le64;
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
                        uint16_t x = *(uint16_t *)(src + j);
                        x          = unpack16(x);

                        uint16_t *dest = va_arg(args, uint16_t *);
                        *dest          = x;

                        break;
                    }

                    case 'i':
                    case 'I':
                    case 'l':
                    case 'L':
                    {
                        uint32_t x = *(uint32_t *)(src + j);
                        x          = unpack32(x);

                        uint32_t *dest = va_arg(args, uint32_t *);
                        *dest          = x;

                        break;
                    }

                    case 'q':
                    case 'Q':
                    {
                        uint64_t x = *(uint64_t *)(src + j);
                        x          = unpack64(x);

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

                        u.f = *(float *)(src + j);
                        u.i = unpack32(u.i);

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

                        u.d = *(double *)(src + j);
                        u.i = unpack64(u.i);

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
    uint8_t o_data[2] = {(uint8_t)(x >> 8), (uint8_t)(x & 0xFF)};

    return *(uint16_t *)o_data;
}

static inline uint16_t __cstruct_pack_le16(uint16_t x)
{
    uint8_t o_data[2] = {(uint8_t)(x & 0xFF), (uint8_t)(x >> 8)};

    return *(uint16_t *)o_data;
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
    uint8_t o_data[4] = {
        (uint8_t)(x >> 24),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)(x & 0xFF),
    };

    return *(uint32_t *)o_data;
}

static inline uint32_t __cstruct_pack_le32(uint32_t x)
{
    uint8_t o_data[4] = {
        (uint8_t)(x & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)(x >> 24),
    };

    return *(uint32_t *)o_data;
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
    uint8_t o_data[8] = {
        (uint8_t)(x >> 56),
        (uint8_t)((x >> 48) & 0xFF),
        (uint8_t)((x >> 40) & 0xFF),
        (uint8_t)((x >> 32) & 0xFF),
        (uint8_t)((x >> 24) & 0xFF),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)(x & 0xFF),
    };

    return *(uint64_t *)o_data;
}

static inline uint64_t __cstruct_pack_le64(uint64_t x)
{
    uint8_t o_data[8] = {
        (uint8_t)(x & 0xFF),
        (uint8_t)((x >> 8) & 0xFF),
        (uint8_t)((x >> 16) & 0xFF),
        (uint8_t)((x >> 24) & 0xFF),
        (uint8_t)((x >> 32) & 0xFF),
        (uint8_t)((x >> 40) & 0xFF),
        (uint8_t)((x >> 48) & 0xFF),
        (uint8_t)(x >> 56),
    };

    return *(uint64_t *)o_data;
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
