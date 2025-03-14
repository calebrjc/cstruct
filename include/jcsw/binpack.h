#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Structs can be specified with a format string similar to those specified in Python's struct
 * module.
 *
 * NOTE: Until a later version, structs and their sizes will be implemented using network byte order
 *       (see: "!" in the Python struct module), though they will be packed from and unpacked into
 *       the native byte order.
 *
 * | Specifier |     Type      | Size |
 * |-----------|---------------|------|
 * |     x     |   nul byte    |  1   |
 * |     b     |   int8_t      |  1   |
 * |     B     |   uint8_t     |  1   |
 * |     h     |   int16_t     |  2   |
 * |     H     |   uint16_t    |  2   |
 * |     i     |   int32_t     |  4   |
 * |     I     |   uint32_t    |  4   |
 * |     l     |   int32_t     |  4   |
 * |     L     |   uint32_t    |  4   |
 * |     q     |   int64_t     |  8   |
 * |     Q     |   uint64_t    |  8   |
 * |     f     |   float       |  4   |
 * |     d     |   double      |  8   |
 * |     s     |   string      |  _   |
 * |     p     | pascal string |  _   | (not implemented yet)
 *
 * NOTE: A multiplier can be appended to a size specifier to indicate repeating that specifier
 *       multiple times.
 */

/// Return the size of a packed struct given its format string.
/// @param [in] format The format string.
/// @return The size of the packed struct, or zero if the format string is invalid.
size_t binpack_sizeof(const char *format);
