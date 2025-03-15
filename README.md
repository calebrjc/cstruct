# `cstruct`

Like the `struct` module in Python, but for C.

> [!IMPORTANT]
> Much of this documentation is taken from the appropriate sections in the
> [Python3 `struct` docs](https://docs.python.org/3/library/struct.html#module-struct) and then
> modified to accurately describe the nature of this library. That being said, assuming that you
> primarily use Python3's `struct` for non-native tasks in an alignment-free context, these changes
> will likely not be too meaningful.

## Synopsis

This library converts C structures between their native machine formats and machine-independent
packed binary blobs. Compact format strings describe the intended conversions to and from native
elements. The main goal of this library is to provide an easier interface for serialization and
deserialization of data for interchange, i.e. networking applications or data storage. The details
governing the specification of format strings can be found below.

## Library Usage

### Format Strings

Format strings describe the data layout when packing and unpacking data. They are built up from
format characters, which specify the type of data being packed and unpacked. In addition, special
characters control the byte order of the blobs. This library will not handle native alignment for
packed blobs, and will pack data precisely as specified in the format string. Each format string
consists of an optional prefix character which describes the endianness of the blob and one or more
format characters which describe the actual data values and padding.

#### Byte Order

The following specifiers may be used to specify the byte ordering of a packed blob:

| Character | Byte Order            |
| :-------: | :-------------------: |
| `<`       | little-endian         |
| `>`       | big-endian            |
| `!`       | network (= big-endian)|

> [!NOTE]
> If no byte ordering is specified, then *network ordering (`!`)* is assumed.

For example, the following code shows the possible byte representations of the number 1023
(`0x3FF` hex) in a packed blob.

```C
#define PACKED_BLOB_LEN 2

void pack_1023(void)
{
    uint8_t packed_be[PACKED_BLOB_LEN] = {0};
    cstruct_pack(">h", packed_be, PACKED_BLOB_LEN, 1023);
    // packed_be == (uint8_t[]){ 0x03, 0xFF };

    uint8_t packed_le[PACKED_BLOB_LEN] = {0};
    cstruct_pack("<h", packed_le, PACKED_BLOB_LEN, 1023);
    // packed_le == (uint8_t[]){ 0xFF, 0x03 };

    uint8_t packed_net[PACKED_BLOB_LEN] = {0};
    cstruct_pack("!h", packed_net, PACKED_BLOB_LEN, 1023);
    // packed_net == (uint8_t[]){ 0x03, 0xFF };
}
```
`cstruct` will handle byte swapping for packing from the native byte order to the specified byte
order, and for unpacking from the specified byte order to the native byte order.

There is no way to indicate non-native byte order (to force byte-swapping); use the appropriate
choice of `<` or `>`.

Since `cstruct` will not add any padding to the packed blob, any desired padding must be specified
by the user (see: the `x` format character).

#### Format Characters

The conversion between their native values and their packed representation should be obvious given
their types.

| Format | C Type            | Size |
| :----: | :---------------: | :--: |
| `x`    | `NUL` byte        | 1    |
| `b`    | `int8_t`          | 1    |
| `B`    | `uint8_t`         | 1    |
| `h`    | `int16_t`         | 2    |
| `H`    | `uint16_t`        | 2    |
| `i`    | `int32_t`         | 4    |
| `I`    | `uint32_t`        | 4    |
| `l`    | `int32_t`         | 4    |
| `L`    | `uint32_t`        | 4    |
| `q`    | `int64_t`         | 8    |
| `Q`    | `uint64_t`        | 8    |
| `f`    | `float`           | 4    |
| `d`    | `double`          | 8    |
| `s`    | `char[]`          |      |

> [!NOTE]
> In a future release, I plan to add support for the `p` format character from Python3's `struct`
> module if it seems to be useful enough.

A format character may be preceded by an integral repeat count. For example, the format string
`"4h"` means exactly the same as `"hhhh"`.

When packing a value `x` using one of the integer formats (`b`, `B`, `h`, `H`, `i`, `I`, `l`, `L`,
`q`, `Q`), if `x` is outside the valid range for that format, then an error is returned.

Since native padding is not supported, multipliers of `0` are not supported and will resolve to
invalid format strings.

### Examples

Example usages and/or application which make use of this library can be found in the `example`
directory.
