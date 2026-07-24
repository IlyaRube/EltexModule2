#include "chmod_parser.h"

#include "permissions.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define WHO_USER  0x01U
#define WHO_GROUP 0x02U
#define WHO_OTHER 0x04U
#define WHO_ALL   (WHO_USER | WHO_GROUP | WHO_OTHER)

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0) {
        snprintf(error, error_size, "%s", message);
    }
}

static int compact_command(const char *source, char *destination,
                           size_t destination_size)
{
    size_t output_index = 0;

    while (*source != '\0') {
        if (!isspace((unsigned char)*source)) {
            if (output_index + 1 >= destination_size) {
                return -1;
            }
            destination[output_index++] = *source;
        }
        source++;
    }

    destination[output_index] = '\0';
    return 0;
}

static unsigned parse_who(const char **cursor)
{
    unsigned who = 0;
    const char *position = *cursor;

    while (*position == 'u' || *position == 'g' ||
           *position == 'o' || *position == 'a') {
        switch (*position) {
        case 'u':
            who |= WHO_USER;
            break;
        case 'g':
            who |= WHO_GROUP;
            break;
        case 'o':
            who |= WHO_OTHER;
            break;
        case 'a':
            who |= WHO_ALL;
            break;
        default:
            break;
        }
        position++;
    }

    *cursor = position;
    return who == 0 ? WHO_ALL : who;
}

static mode_t class_bits_from_rwx(unsigned rwx, unsigned who)
{
    mode_t result = 0;

    if ((who & WHO_USER) != 0) {
        result |= (mode_t)(rwx << 6);
    }
    if ((who & WHO_GROUP) != 0) {
        result |= (mode_t)(rwx << 3);
    }
    if ((who & WHO_OTHER) != 0) {
        result |= (mode_t)rwx;
    }

    return result;
}

static unsigned class_value(mode_t mode, char source_class)
{
    switch (source_class) {
    case 'u':
        return (unsigned)((mode >> 6) & 07);
    case 'g':
        return (unsigned)((mode >> 3) & 07);
    case 'o':
        return (unsigned)(mode & 07);
    default:
        return 0;
    }
}

static mode_t target_mask(unsigned who)
{
    mode_t mask = 0;

    if ((who & WHO_USER) != 0) {
        mask |= 0700;
    }
    if ((who & WHO_GROUP) != 0) {
        mask |= 0070;
    }
    if ((who & WHO_OTHER) != 0) {
        mask |= 0007;
    }

    return mask;
}

static int parse_permission_sequence(const char **cursor, mode_t mode,
                                     unsigned who, mode_t *permission_bits,
                                     int *has_permissions)
{
    const char *position = *cursor;
    mode_t bits = 0;
    int found = 0;

    while (*position != '\0' && *position != ',' &&
           *position != '+' && *position != '-' && *position != '=') {
        unsigned rwx;

        switch (*position) {
        case 'r':
            bits |= class_bits_from_rwx(04U, who);
            break;
        case 'w':
            bits |= class_bits_from_rwx(02U, who);
            break;
        case 'x':
            bits |= class_bits_from_rwx(01U, who);
            break;
        case 'u':
        case 'g':
        case 'o':
            rwx = class_value(mode, *position);
            bits |= class_bits_from_rwx(rwx, who);
            break;
        default:
            return -1;
        }
        found = 1;
        position++;
    }

    *cursor = position;
    *permission_bits = bits;
    *has_permissions = found;
    return 0;
}

static int apply_symbolic(mode_t current, const char *command,
                          mode_t *result, char *error, size_t error_size)
{
    const char *cursor = command;
    mode_t working = current & PERMISSIONS_MASK;

    while (*cursor != '\0') {
        unsigned who = parse_who(&cursor);
        int action_count = 0;

        while (*cursor == '+' || *cursor == '-' || *cursor == '=') {
            char operation = *cursor++;
            mode_t permission_bits;
            int has_permissions;

            if (parse_permission_sequence(&cursor, working, who,
                                          &permission_bits,
                                          &has_permissions) != 0) {
                set_error(error, error_size,
                          "Неизвестный символ в chmod-команде");
                return -1;
            }

            if (!has_permissions && operation != '=') {
                set_error(error, error_size,
                          "После + или - должны быть указаны права");
                return -1;
            }

            switch (operation) {
            case '+':
                working |= permission_bits;
                break;
            case '-':
                working &= (mode_t)~permission_bits;
                break;
            case '=':
                working &= (mode_t)~target_mask(who);
                working |= permission_bits;
                break;
            default:
                break;
            }

            working &= PERMISSIONS_MASK;
            action_count++;
        }

        if (action_count == 0) {
            set_error(error, error_size,
                      "Ожидался оператор +, - или =");
            return -1;
        }

        if (*cursor == ',') {
            cursor++;
            if (*cursor == '\0') {
                set_error(error, error_size,
                          "После запятой отсутствует chmod-команда");
                return -1;
            }
        } else if (*cursor != '\0') {
            set_error(error, error_size,
                      "Некорректный формат chmod-команды");
            return -1;
        }
    }

    *result = working;
    return 0;
}

int chmod_apply(mode_t current, const char *command, mode_t *result,
                char *error, size_t error_size)
{
    char compact[256];
    mode_t direct_mode;

    if (command == NULL || result == NULL) {
        set_error(error, error_size, "Передан пустой указатель");
        return -1;
    }

    if (compact_command(command, compact, sizeof(compact)) != 0) {
        set_error(error, error_size, "Команда слишком длинная");
        return -1;
    }

    if (compact[0] == '\0') {
        set_error(error, error_size, "Пустая chmod-команда");
        return -1;
    }

    if (permissions_parse(compact, &direct_mode, NULL, 0) == 0) {
        *result = direct_mode;
        return 0;
    }

    return apply_symbolic(current, compact, result, error, error_size);
}
