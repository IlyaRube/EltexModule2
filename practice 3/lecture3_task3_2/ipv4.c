#include "ipv4.h"

#include <stdio.h>

int ipv4_parse(const char *text, uint32_t *address)
{
    unsigned int octets[4];
    char extra;

    if (text == NULL || address == NULL) {
        return -1;
    }

    /*
     * Символ extra нужен, чтобы отклонить строки с лишними данными,
     * например "192.168.1.1abc" или "192.168.1.1.5".
     */
    if (sscanf(text,
               "%u.%u.%u.%u%c",
               &octets[0],
               &octets[1],
               &octets[2],
               &octets[3],
               &extra) != 4) {
        return -1;
    }

    for (size_t i = 0; i < 4; ++i) {
        if (octets[i] > 255U) {
            return -1;
        }
    }

    *address = ((uint32_t)octets[0] << 24)
             | ((uint32_t)octets[1] << 16)
             | ((uint32_t)octets[2] << 8)
             | (uint32_t)octets[3];

    return 0;
}

void ipv4_format(uint32_t address, char result[IPV4_STRING_SIZE])
{
    if (result == NULL) {
        return;
    }

    (void)snprintf(result,
                   IPV4_STRING_SIZE,
                   "%u.%u.%u.%u",
                   (unsigned int)((address >> 24) & 0xFFU),
                   (unsigned int)((address >> 16) & 0xFFU),
                   (unsigned int)((address >> 8) & 0xFFU),
                   (unsigned int)(address & 0xFFU));
}

int ipv4_mask_is_valid(uint32_t mask)
{
    int zero_seen = 0;

    for (int bit = 31; bit >= 0; --bit) {
        const int current_bit = (int)((mask >> bit) & 1U);

        if (current_bit == 0) {
            zero_seen = 1;
        } else if (zero_seen != 0) {
            /*
             * После первого нуля встретилась единица.
             * Значит, маска разорвана и некорректна.
             */
            return 0;
        }
    }

    return 1;
}

unsigned int ipv4_mask_prefix_length(uint32_t mask)
{
    unsigned int length = 0;

    while ((mask & 0x80000000U) != 0U) {
        ++length;
        mask <<= 1;
    }

    return length;
}

int ipv4_same_subnet(uint32_t first, uint32_t second, uint32_t mask)
{
    return (first & mask) == (second & mask);
}
