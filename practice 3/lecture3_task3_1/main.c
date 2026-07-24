#include "chmod_parser.h"
#include "file_info.h"
#include "permissions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_SIZE 512
#define ERROR_SIZE 256

static int read_line(const char *prompt, char *buffer, size_t buffer_size)
{
    size_t length;
    int extra_character;

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        while ((extra_character = getchar()) != '\n' &&
               extra_character != EOF) {
        }
    }

    return 1;
}

static void print_permissions(mode_t mode)
{
    char symbolic[PERMISSIONS_SYMBOLIC_SIZE];
    char bits[PERMISSIONS_BITS_SIZE];
    char octal[PERMISSIONS_OCTAL_SIZE];

    permissions_to_symbolic(mode, symbolic);
    permissions_to_bits(mode, bits);
    permissions_to_octal(mode, octal);

    printf("Буквенное представление: %s\n", symbolic);
    printf("Цифровое представление: %s\n", octal);
    printf("Битовое представление:   %s\n", bits);
}

static void show_menu(void)
{
    printf("\n=== Расчёт маски прав доступа ===\n");
    printf("1. Ввести права доступа\n");
    printf("2. Получить права доступа файла через stat()\n");
    printf("3. Изменить текущую маску командой chmod\n");
    printf("4. Показать текущую маску\n");
    printf("0. Выход\n");
}

int main(void)
{
    char input[INPUT_SIZE];
    char error[ERROR_SIZE];
    mode_t current_mode = 0;
    int has_current_mode = 0;

    for (;;) {
        char *end_pointer;
        long choice;

        show_menu();
        if (!read_line("Выберите пункт: ", input, sizeof(input))) {
            printf("\nВвод завершён.\n");
            break;
        }

        choice = strtol(input, &end_pointer, 10);
        if (end_pointer == input || *end_pointer != '\0') {
            printf("Ошибка: введите номер пункта меню.\n");
            continue;
        }

        if (choice == 0) {
            break;
        }

        if (choice == 1) {
            mode_t parsed_mode;

            if (!read_line(
                    "Введите права (755, 0755 или rwxr-xr-x): ",
                    input, sizeof(input))) {
                break;
            }

            if (permissions_parse(input, &parsed_mode,
                                  error, sizeof(error)) != 0) {
                printf("Ошибка: %s\n", error);
                continue;
            }

            current_mode = parsed_mode;
            has_current_mode = 1;
            print_permissions(current_mode);
            continue;
        }

        if (choice == 2) {
            FileInfo info;
            char symbolic[PERMISSIONS_SYMBOLIC_SIZE];

            if (!read_line("Введите имя или путь к файлу: ",
                           input, sizeof(input))) {
                break;
            }

            if (file_info_read(input, &info, error, sizeof(error)) != 0) {
                printf("Ошибка: %s\n", error);
                continue;
            }

            current_mode = info.permissions;
            has_current_mode = 1;
            permissions_to_symbolic(current_mode, symbolic);

            printf("Представление как в первом поле ls -l: %c%s\n",
                   info.type_symbol, symbolic);
            print_permissions(current_mode);
            printf("Для ручного сравнения выполните: ls -l -- \"%s\"\n",
                   input);
            continue;
        }

        if (choice == 3) {
            mode_t changed_mode;

            if (!has_current_mode) {
                printf("Сначала задайте права вручную или загрузите их "
                       "из файла.\n");
                continue;
            }

            printf("Примеры: u+x, g-w, o=r, ug+wx, g=u, "
                   "u+r-w, u+x,g-w,o=r, 755\n");
            if (!read_line("Введите chmod-команду: ",
                           input, sizeof(input))) {
                break;
            }

            if (chmod_apply(current_mode, input, &changed_mode,
                            error, sizeof(error)) != 0) {
                printf("Ошибка: %s\n", error);
                continue;
            }

            current_mode = changed_mode;
            printf("Новая маска сохранена только в памяти. "
                   "Файл не изменён.\n");
            print_permissions(current_mode);
            continue;
        }

        if (choice == 4) {
            if (!has_current_mode) {
                printf("Текущая маска ещё не задана.\n");
            } else {
                print_permissions(current_mode);
            }
            continue;
        }

        printf("Ошибка: такого пункта меню нет.\n");
    }

    return 0;
}
