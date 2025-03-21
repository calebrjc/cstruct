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

/**
 * @brief Serializes values into a buffer according to a format string.
 *
 * This function packs the provided values into the given buffer following the layout
 * defined by the format string. The format string may begin with a byte-order specifier
 * ('<' for little-endian, '!' or '>' for big-endian, with big-endian as the default), and
 * supports optional multipliers for repeating data fields.
 *
 * Supported format characters include:
 * - 'b'/'B': 8-bit integers.
 * - 'h'/'H': 16-bit integers.
 * - 'i'/'I', 'l'/'L': 32-bit integers.
 * - 'q'/'Q': 64-bit integers.
 * - 'f': 32-bit floating-point values.
 * - 'd': 64-bit double-precision values.
 * - 's': Strings, with unused bytes set to zero.
 * - 'x': Padding bytes (zero-filled).
 *
 * The function iteratively processes the format string and packs each value into the buffer
 * using the appropriate conversion based on type and endianness. If an invalid format, insufficient
 * buffer space, or invalid multiplier is encountered, the function aborts and returns -1.
 *
 * @param format The format string specifying the layout and types for serialization.
 * @param buffer The destination buffer where the packed data will be stored.
 * @param buffer_size The size of the destination buffer in bytes.
 * @param ... A variable list of values that correspond to the format specifiers.
 *
 * @return ssize_t The total number of bytes packed into the buffer on success, or -1 on error.
 */

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

/**
 * @brief Unpacks serialized data from a buffer according to a format string.
 *
 * This function reads data sequentially from a given buffer and stores the unpacked values into
 * variables provided as additional arguments. The format string specifies the layout, endianness,
 * and data types of the serialized data. An optional initial endianness specifier ('!', '<', '>') in
 * the format string sets the byte order (default is big-endian), and numeric type specifiers (such as
 * 'b', 'h', 'i', 'q', 'f', 'd') determine how subsequent bytes are processed. A multiplier may precede
 * a type to indicate multiple consecutive elements, while a specifier of 'x' skips (pads) bytes.
 *
 * The function returns the total number of bytes read from the buffer. If an error occurs due to an
 * empty or invalid format string, an invalid multiplier, an unrecognized type specifier, or an attempt
 * to read beyond the buffer size, the function returns -1.
 *
 * @param format Format string defining the data layout. May start with an endianness indicator followed
 *               by type characters, optionally preceded by multipliers.
 * @param buffer Pointer to the source buffer containing serialized data.
 * @param buffer_size Total size of the source buffer in bytes.
 *
 * @return ssize_t Total number of bytes read from the buffer on success, or -1 if an error occurs.
 *
 * @note The variable arguments must be pointers that match the expected data types derived from the format string.
 */
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

/**
 * @brief Computes the total size in bytes of a data structure based on a format string.
 *
 * The function iterates over the format string—optionally skipping an initial byte order specifier
 * ('!', '<', or '>')—and calculates the cumulative size by processing each format character along
 * with its associated multiplier. It returns the total size in bytes, or -1 if the format string is
 * NULL, empty, contains an invalid multiplier, or references an unrecognized type.
 *
 * @param format A null-terminated string that specifies the structure layout.
 * @return ssize_t Total size in bytes on success, or -1 on error.
 */
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

/**
 * @brief Parses a numeric multiplier from a format string.
 *
 * This function reads consecutive digit characters from the given format string starting at
 * the index pointed to by *i, converts them into a positive integer multiplier, and advances
 * the index past the digits. If the character at the initial index is not a digit, it returns a
 * default multiplier of 1. In the case where the resulting multiplier is not positive (e.g., due
 * to an overflow), the function returns -1.
 *
 * @note Assumes that the format string is non-NULL and non-empty and that the index pointer is valid.
 *
 * @param format The format string containing the multiplier.
 * @param i Pointer to the current index in the format string; updated to point to the first character
 *          after the multiplier digits.
 * @return int32_t The parsed multiplier, 1 if no digits are found, or -1 if the multiplier is invalid.
 */
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

/**
 * @brief Computes the total byte size for a specified format type.
 *
 * This function determines the base size in bytes associated with a given format character and scales that
 * size by the provided multiplier. Supported format characters are:
 * - 'x', 'b', 'B', 's': 1 byte.
 * - 'h', 'H': 2 bytes.
 * - 'i', 'I', 'l', 'L', 'f': 4 bytes.
 * - 'q', 'Q', 'd': 8 bytes.
 *
 * @param c The format character representing the data type.
 * @param multiplier The factor by which the base size is multiplied.
 * @return ssize_t The total size in bytes (base size multiplied by multiplier), or -1 if the format character is invalid.
 */
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

/**
 * @brief Converts a 16-bit integer to its big-endian representation.
 *
 * Reorders the bytes of the provided 16-bit integer so that the most significant byte comes first,
 * ensuring the value is represented in big-endian format. This is useful for network transmission or
 * file storage where big-endian ordering is required.
 *
 * @param x The 16-bit integer to be converted.
 * @return uint16_t The big-endian representation of the input value.
 */
static inline uint16_t __cstruct_pack_be16(uint16_t x)
{
    uint8_t o_data[2] = {(uint8_t)(x >> 8), (uint8_t)(x & 0xFF)};

    return *(uint16_t *)o_data;
}

/**
 * @brief Converts a 16-bit integer to its little-endian representation.
 *
 * This inline function rearranges the bytes of the input integer so that the lower-order byte
 * is placed first, resulting in a little-endian formatted value.
 *
 * @param x The 16-bit integer to convert.
 * @return uint16_t The little-endian representation of the input integer.
 */
static inline uint16_t __cstruct_pack_le16(uint16_t x)
{
    uint8_t o_data[2] = {(uint8_t)(x & 0xFF), (uint8_t)(x >> 8)};

    return *(uint16_t *)o_data;
}

/**
 * @brief Converts a 16-bit integer from big-endian byte order to host order.
 *
 * This inline function reassembles a 16-bit integer from its big-endian
 * byte representation by shifting the first byte by 8 bits and combining it
 * with the second byte.
 *
 * @param x A 16-bit integer with big-endian byte ordering.
 * @return uint16_t The value converted to host byte order.
 */
static inline uint16_t __cstruct_unpack_be16(uint16_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint16_t)data[0] << 8) | data[1];
}

/**
 * @brief Unpacks a 16-bit integer from its little-endian representation.
 *
 * This function converts a 16-bit integer stored in little-endian byte order
 * to the host's native byte order by reordering its bytes.
 *
 * @param x The 16-bit value in little-endian format.
 * @return The corresponding 16-bit value in host byte order.
 */
static inline uint16_t __cstruct_unpack_le16(uint16_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint16_t)data[1] << 8) | data[0];
}

/**
 * @brief Packs a 32-bit unsigned integer into big-endian format.
 *
 * This function converts the provided 32-bit integer into a big-endian
 * representation by rearranging its bytes and returning the resulting value.
 *
 * @param x The 32-bit unsigned integer to be packed.
 * @return A 32-bit unsigned integer with its bytes ordered in big-endian format.
 */
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

/**
 * @brief Packs a 32-bit unsigned integer into little-endian byte order.
 *
 * This inline function reorders the bytes of a 32-bit unsigned integer so that the
 * least significant byte is stored at the lowest address, producing a little-endian
 * representation of the input value.
 *
 * @param x The 32-bit unsigned integer to be converted.
 * @return uint32_t The input integer represented in little-endian format.
 */
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

/**
 * @brief Converts a 32-bit big-endian integer to host byte order.
 *
 * This function interprets the provided 32-bit integer as a sequence of four bytes stored in big-endian order
 * and reconstructs it into a native 32-bit integer using bitwise operations.
 *
 * @param x The 32-bit integer whose bytes are in big-endian order.
 * @return The 32-bit integer converted to host byte order.
 */
static inline uint32_t __cstruct_unpack_be32(uint32_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8)
         | data[3];
}

/**
 * @brief Converts a 32-bit integer from little-endian byte order to host order.
 *
 * This function reorders the bytes of a 32-bit integer provided in little-endian format,
 * returning the correctly ordered value for the host system.
 *
 * @param x The 32-bit integer in little-endian byte order.
 * @return The 32-bit integer converted to host byte order.
 */
static inline uint32_t __cstruct_unpack_le32(uint32_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8)
         | data[0];
}

/**
 * @brief Converts a 64-bit unsigned integer to big-endian format.
 *
 * This function reorders the bytes of the provided integer to represent it in big-endian (network byte order).
 *
 * @param x A 64-bit unsigned integer in host byte order.
 * @return The integer represented in big-endian format.
 */
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

/**
 * @brief Packs a 64-bit integer into its little-endian representation.
 *
 * This function converts the input 64-bit integer into little-endian byte order by
 * manually assembling its bytes, ensuring the least significant byte is stored first.
 *
 * @param x The 64-bit integer to be packed.
 * @return uint64_t The little-endian representation of the input integer.
 */
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

/**
 * @brief Converts a 64-bit integer from big-endian to host byte order.
 *
 * This inline function treats the input as a sequence of eight bytes in big-endian order
 * and recombines them into a 64-bit integer in the native host byte order.
 *
 * @param x A 64-bit integer represented in big-endian format.
 * @return uint64_t The 64-bit integer converted to host byte order.
 */
static inline uint64_t __cstruct_unpack_be64(uint64_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) | ((uint64_t)data[2] << 40)
         | ((uint64_t)data[3] << 32) | ((uint64_t)data[4] << 24) | ((uint64_t)data[5] << 16)
         | ((uint64_t)data[6] << 8) | data[7];
}

/**
 * @brief Converts a 64-bit integer from little-endian to host byte order.
 *
 * This function reorders the bytes of the provided 64-bit value, which is assumed
 * to be stored in little-endian format, so that it is represented correctly on the host machine.
 *
 * @param x 64-bit integer in little-endian format.
 * @return uint64_t The 64-bit integer in host byte order.
 */
static inline uint64_t __cstruct_unpack_le64(uint64_t x)
{
    uint8_t *data = (uint8_t *)&x;
    return ((uint64_t)data[7] << 56) | ((uint64_t)data[6] << 48) | ((uint64_t)data[5] << 40)
         | ((uint64_t)data[4] << 32) | ((uint64_t)data[3] << 24) | ((uint64_t)data[2] << 16)
         | ((uint64_t)data[1] << 8) | data[0];
}
