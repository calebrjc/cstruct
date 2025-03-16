#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cstruct.h"

#define GAME_PACKET_HEADER_FORMAT "!HBBIIHBx16s3f3hBB"

typedef struct
{
    uint16_t magic;          // Magic number to identify the protocol (0xB00B)
    uint8_t  version;        // Protocol version
    uint8_t  packet_type;    // Type of packet (login, update, chat, etc.)
    uint32_t sequence_num;   // Sequence number for ordering packets
    uint32_t timestamp;      // Timestamp when packet was sent
    uint16_t payload_length; // Length of the payload data
    uint8_t  flags;          // Bit flags for various options
    uint8_t  reserved;       // Reserved for future use (padding)
    char     session_id[16]; // Session identifier
    float    position[3];    // Player position (x, y, z)
    int16_t  rotation[3];    // Player rotation (pitch, yaw, roll) in degrees
    uint8_t  health;         // Player health percentage
    uint8_t  checksum;       // Simple checksum for header integrity
} game_packet_header_t;

static const game_packet_header_t test_packet_header = {
    .magic          = 0xB00B,                     // Magic number
    .version        = 0x02,                       // Protocol version 2
    .packet_type    = 0x05,                       // Packet type 5 (player update)
    .sequence_num   = 1234567,                    // Sequence number
    .timestamp      = 1620000000,                 // Unix timestamp (May 3, 2021)
    .payload_length = 512,                        // 512 bytes of payload
    .flags          = 0x0A,                       // Flags: 0x0A (encrypted | compressed)
    .reserved       = 0x00,                       // Reserved byte (should be zero)
    .session_id     = "ABCD1234EFGH5678",         // 16-character session ID
    .position       = {128.5f, -42.75f, 1024.0f}, // Player position (x, y, z)
    .rotation       = {45, 180, -30},             // Player rotation (pitch, yaw, roll)
    .health         = 75,                         // 75% health
    .checksum       = 0xCC                        // Checksum value
};

void __hexdump(uint8_t *buffer, size_t size);

int main(int argc, char **argv)
{
    uint8_t buffer[1024];

    ssize_t packed_size = cstruct_pack(
        GAME_PACKET_HEADER_FORMAT,
        buffer,
        sizeof(buffer),
        test_packet_header.magic,
        test_packet_header.version,
        test_packet_header.packet_type,
        test_packet_header.sequence_num,
        test_packet_header.timestamp,
        test_packet_header.payload_length,
        test_packet_header.flags,
        test_packet_header.session_id,
        test_packet_header.position[0],
        test_packet_header.position[1],
        test_packet_header.position[2],
        test_packet_header.rotation[0],
        test_packet_header.rotation[1],
        test_packet_header.rotation[2],
        test_packet_header.health,
        test_packet_header.checksum);

    game_packet_header_t unpacked_packet_header = {0};

    ssize_t unpacked_size = cstruct_unpack(
        GAME_PACKET_HEADER_FORMAT,
        buffer,
        packed_size,
        &unpacked_packet_header.magic,
        &unpacked_packet_header.version,
        &unpacked_packet_header.packet_type,
        &unpacked_packet_header.sequence_num,
        &unpacked_packet_header.timestamp,
        &unpacked_packet_header.payload_length,
        &unpacked_packet_header.flags,
        unpacked_packet_header.session_id,
        &unpacked_packet_header.position[0],
        &unpacked_packet_header.position[1],
        &unpacked_packet_header.position[2],
        &unpacked_packet_header.rotation[0],
        &unpacked_packet_header.rotation[1],
        &unpacked_packet_header.rotation[2],
        &unpacked_packet_header.health,
        &unpacked_packet_header.checksum);

    printf("test_packet_header:\n");
    __hexdump((uint8_t *)&test_packet_header, sizeof(test_packet_header));
    printf("\npacked buffer:\n");
    __hexdump(buffer, packed_size);
    printf("\nunpacked_packet_header:\n");
    __hexdump((uint8_t *)&unpacked_packet_header, sizeof(unpacked_packet_header));

    return 0;
}

void __hexdump(uint8_t *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%02x ", buffer[i]);

        if ((i + 1) % 8 == 0)
        {
            printf(" ");
        }

        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }

    if (size % 16 != 0)
    {
        printf("\n");
    }
}
