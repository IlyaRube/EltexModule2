#include "permissions.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "%s", message);
    }
}

static void trim_copy(const char *source, char *destination,
                      size_t destination_size)
{
    const char *begin = source;
    const char *end;
    size_t length;

    while (*begin != '\0' && isspace((unsigned char)*begin)) {
        begin++;
    }

    end = begin + strlen(begin);
    while (end > begin && isspace((unsigned char)end[-1])) {
        end--;
    }

    length = (size_t)(end - begin);
    if (length >= destination_size) {
        length = destination_size - 1;
    }

    memcpy(destination, begin, length);
    destination[length] = '\0';
}

static int parse_octal(const char *text, mode_t *result)
{
    size_t length = strlen(text);
    size_t start = 0;
    unsigned value = 0;
    size_t index;

    if (length == 4 && text[0] == '0') {
        start = 1;
    } else if (length != 3) {
        return -1;
    }

    for (index = start; index < length; index++) {
        if (text[index] < '0' || text[index] > '7') {
            return -1;
        }
        value = value * 8U + (unsigned)(text[index] - '0');
    }

    *result = (mode_t)(value & 0777U);
    return 0;
}

static int parse_symbolic(const char *text, mode_t *result)
{
    const char *permissions = text;
    size_t length = strlen(text);
    mode_t mode = 0;
    size_t index;
    static const mode_t bits[9] = {
        0400, 0200, 0100,
        0040, 0020, 0010,
        0004, 0002, 0001
    };
    static const char expected[9] = {
        'r', 'w', 'x',
        'r', 'w', 'x',
        'r', 'w', 'x'
    };

    if (length == 10) {
        permissions = text + 1;
        length = 9;
    }

    if (length != 9) {
        return -1;
    }

    for (index = 0; index < 9; index++) {
        if (permissions[index] == expected[index]) {
            mode |= bits[index];
        } else if (permissions[index] != '-') {
            return -1;
        }
    }

    *result = mode;
    return 0;
}

int permissions_parse(const char *text, mode_t *result,
                      char *error, size_t error_size)
{
    char buffer[128];

    if (text == NULL || result == NULL) {
        set_error(error, error_size, "Передан пустой указатель");
        return -1;
    }

    trim_copy(text, buffer, sizeof(buffer));
    if (buffer[0] == '\0') {
        set_error(error, error_size, "Пустая строка прав доступа");
        return -1;
    }

    if (parse_octal(buffer, result) == 0 ||
        parse_symbolic(buffer, result) == 0) {
        *result &= PERMISSIONS_MASK;
        return 0;
    }

    set_error(error, error_size,
              "Ожидалось 755, 0755, rwxr-xr-x или -rwxr-xr-x");
    return -1;
}

void permissions_to_symbolic(mode_t mode,
                             char result[PERMISSIONS_SYMBOLIC_SIZE])
{
    static const mode_t bits[9] = {
        0400, 0200, 0100,
        0040, 0020, 0010,
        0004, 0002, 0001
    };
    static const char symbols[9] = {
        'r', 'w', 'x',
        'r', 'w', 'x',
        'r', 'w', 'x'
    };
    size_t index;

    mode &= PERMISSIONS_MASK;
    for (index = 0; index < 9; index++) {
        result[index] = (mode & bits[index]) != 0 ? symbols[index] : '-';
    }
    result[9] = '\0';
}

void permissions_to_bits(mode_t mode,
                         char result[PERMISSIONS_BITS_SIZE])
{
    int source_bit;
    size_t output_index = 0;

    mode &= PERMISSIONS_MASK;
    for (source_bit = 8; source_bit >= 0; source_bit--) {
        result[output_index++] =
            (mode & ((mode_t)1U << source_bit)) != 0 ? '1' : '0';

        if (source_bit == 6 || source_bit == 3) {
            result[output_index++] = ' ';
        }
    }
    result[output_index] = '\0';
}

void permissions_to_octal(mode_t mode,
                          char result[PERMISSIONS_OCTAL_SIZE])
{
    snprintf(result, PERMISSIONS_OCTAL_SIZE, "%03o",
             (unsigned)(mode & PERMISSIONS_MASK));
}
