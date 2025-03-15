#pragma once

#include <stddef.h>

/// Return the size of a packed struct given its format string.
/// @param[in] format The format string.
/// @return The size of the packed struct, or zero if the format string is invalid.
size_t cstruct_sizeof(const char *format);
