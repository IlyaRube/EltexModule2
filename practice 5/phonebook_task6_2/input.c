#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void discard_line_tail(void)
{
    int ch;

    do {
        ch = getchar();
    } while (ch != '\n' && ch != EOF);
}

void input_trim(char *text)
{
    char *start;
    size_t length;

    if (text == NULL) {
        return;
    }

    start = text;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    length = strlen(text);
    while (length > 0 && isspace((unsigned char)text[length - 1])) {
        text[length - 1] = '\0';
        length--;
    }
}

int input_read_line(const char *prompt, char *buffer, size_t buffer_size)
{
    size_t length;

    if (buffer == NULL || buffer_size == 0) {
        return 0;
    }

    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else if (length + 1 == buffer_size) {
        discard_line_tail();
    }

    input_trim(buffer);
    return 1;
}

int input_read_int_range(const char *prompt, int min_value, int max_value, int *value)
{
    char line[128];
    char *end;
    long parsed;

    if (value == NULL || min_value > max_value) {
        return 0;
    }

    for (;;) {
        if (!input_read_line(prompt, line, sizeof(line))) {
            return 0;
        }

        errno = 0;
        end = NULL;
        parsed = strtol(line, &end, 10);

        if (errno == 0 && end != line && *end == '\0' &&
            parsed >= min_value && parsed <= max_value &&
            parsed >= INT_MIN && parsed <= INT_MAX) {
            *value = (int)parsed;
            return 1;
        }

        printf("Ошибка: введите целое число от %d до %d.\n", min_value, max_value);
    }
}
