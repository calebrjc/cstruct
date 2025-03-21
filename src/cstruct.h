#pragma once

#include <stddef.h>
#include <sys/types.h>

/// Pack values into a binary blob according to the format string.
/// @param[in] format The format string describing the data layout.
/// @param[out] buffer The buffer to pack the data into.
/// @param[in] buffer_size The length of the buffer.
/// @param[in] ... The values to pack, corresponding to the format string.
/// @return The number of bytes packed, or -1 if an error occurred.
ssize_t cstruct_pack(const char *format, void *buffer, size_t buffer_size, ...);

/// Unpack values from a binary blob according to the format string.
/// @param[in] format The format string describing the data layout.
/// @param[in] buffer The buffer to unpack the data from.
/// @param[in] buffer_size The length of the buffer.
/// @param[out] ... Pointers to variables where the unpacked values will be stored.
/// @return The number of bytes unpacked, or -1 if an error occurred.
ssize_t cstruct_unpack(const char *format, const void *buffer, size_t buffer_size, ...);

/// Return the size of a packed struct given its format string.
/// @param[in] format The format string.
/// @return The size of the packed struct, or -1 if the format string is invalid.
ssize_t cstruct_sizeof(const char *format);
