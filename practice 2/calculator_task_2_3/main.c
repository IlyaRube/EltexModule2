#include "calculator.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define INPUT_BUFFER_SIZE 256

static void console_vprintf(const char *format, va_list arguments)
{
    va_list arguments_copy;
    int required_size;
    char *utf8_text;

    va_copy(arguments_copy, arguments);
    required_size = vsnprintf(NULL, 0, format, arguments_copy);
    va_end(arguments_copy);

    if (required_size < 0) {
        return;
    }

    utf8_text = malloc((size_t)required_size + 1U);

    if (utf8_text == NULL) {
        return;
    }

    vsnprintf(utf8_text, (size_t)required_size + 1U, format, arguments);

#ifdef _WIN32
    {
        HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD console_mode;

        if (output_handle != INVALID_HANDLE_VALUE &&
            output_handle != NULL &&
            GetConsoleMode(output_handle, &console_mode)) {
            int wide_size = MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                utf8_text,
                required_size,
                NULL,
                0
            );

            if (wide_size > 0) {
                wchar_t *wide_text = malloc(
                    (size_t)wide_size * sizeof(*wide_text)
                );

                if (wide_text != NULL) {
                    DWORD written = 0;

                    MultiByteToWideChar(
                        CP_UTF8,
                        MB_ERR_INVALID_CHARS,
                        utf8_text,
                        required_size,
                        wide_text,
                        wide_size
                    );

                    WriteConsoleW(
                        output_handle,
                        wide_text,
                        (DWORD)wide_size,
                        &written,
                        NULL
                    );

                    free(wide_text);
                    free(utf8_text);
                    return;
                }
            }
        }
    }
#endif

    fwrite(utf8_text, 1U, (size_t)required_size, stdout);
    fflush(stdout);
    free(utf8_text);
}

static void console_printf(const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    console_vprintf(format, arguments);
    va_end(arguments);
}

static void configure_console(void)
{
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");

#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
#endif
}

static void discard_remaining_input(void)
{
    int ch;

    do {
        ch = getchar();
    } while (ch != '\n' && ch != EOF);
}

static int read_line(char *buffer, size_t buffer_size)
{
    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return 0;
    }

    if (strchr(buffer, '\n') == NULL) {
        discard_remaining_input();
    }

    return 1;
}

static int parse_int(const char *text, int *value)
{
    char *end;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);

    if (text == end || errno == ERANGE || parsed < INT_MIN ||
        parsed > INT_MAX) {
        return 0;
    }

    while (isspace((unsigned char)*end)) {
        end++;
    }

    if (*end != '\0') {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static int parse_double(const char *text, double *value)
{
    char *end;
    double parsed;

    errno = 0;
    parsed = strtod(text, &end);

    if (text == end || errno == ERANGE) {
        return 0;
    }

    while (isspace((unsigned char)*end)) {
        end++;
    }

    if (*end != '\0') {
        return 0;
    }

    *value = parsed;
    return 1;
}

static int read_int(const char *prompt, int *value)
{
    char buffer[INPUT_BUFFER_SIZE];

    for (;;) {
        console_printf("%s", prompt);

        if (!read_line(buffer, sizeof(buffer))) {
            return 0;
        }

        if (parse_int(buffer, value)) {
            return 1;
        }

        console_printf("Ошибка: введите целое число.\n");
    }
}

static int read_double(const char *prompt, double *value)
{
    char buffer[INPUT_BUFFER_SIZE];

    for (;;) {
        console_printf("%s", prompt);

        if (!read_line(buffer, sizeof(buffer))) {
            return 0;
        }

        if (parse_double(buffer, value)) {
            return 1;
        }

        console_printf(
            "Ошибка: введите число. Для дробной части используйте точку.\n"
        );
    }
}

static CalculatorStatus register_default_commands(CalculatorMenu *menu)
{
    CalculatorStatus status;

    status = calculator_menu_add(menu, 1, "Сложение", calculator_add);
    if (status != CALCULATOR_OK) {
        return status;
    }

    status = calculator_menu_add(menu, 2, "Вычитание", calculator_subtract);
    if (status != CALCULATOR_OK) {
        return status;
    }

    status = calculator_menu_add(menu, 3, "Умножение", calculator_multiply);
    if (status != CALCULATOR_OK) {
        return status;
    }

    return calculator_menu_add(menu, 4, "Деление", calculator_divide);
}

static void print_menu(const CalculatorMenu *menu)
{
    size_t index;

    console_printf("\n");
    console_printf("========================================\n");
    console_printf("      ДИНАМИЧЕСКИЙ КАЛЬКУЛЯТОР          \n");
    console_printf("========================================\n");

    for (index = 0U; index < menu->count; index++) {
        const CalculatorCommand *command = calculator_menu_get(menu, index);

        if (command != NULL) {
            console_printf("%d. %s\n", command->id, command->name);
        }
    }

    console_printf("0. Выход\n");
    console_printf("========================================\n");
}

static void print_calculation_error(CalculatorStatus status)
{
    if (status == CALCULATOR_DIVISION_BY_ZERO) {
        console_printf("Ошибка: деление на ноль невозможно.\n");
    } else if (status == CALCULATOR_NULL_RESULT) {
        console_printf("Внутренняя ошибка: не задан адрес результата.\n");
    } else {
        console_printf("Внутренняя ошибка вычисления.\n");
    }
}

int main(void)
{
    CalculatorMenu menu;
    CalculatorStatus status;
    int choice;

    configure_console();
    calculator_menu_init(&menu);

    status = register_default_commands(&menu);

    if (status != CALCULATOR_OK) {
        console_printf("Не удалось сформировать список команд.\n");
        calculator_menu_destroy(&menu);
        return 1;
    }

    for (;;) {
        const CalculatorCommand *command;
        double first;
        double second;
        double result;

        print_menu(&menu);

        if (!read_int("Выберите действие: ", &choice)) {
            console_printf("\nВвод завершён. Программа остановлена.\n");
            break;
        }

        if (choice == 0) {
            console_printf("Работа калькулятора завершена.\n");
            break;
        }

        command = calculator_menu_find(&menu, choice);

        if (command == NULL) {
            console_printf("Ошибка: такого пункта меню нет.\n");
            continue;
        }

        if (!read_double("Введите первое число: ", &first) ||
            !read_double("Введите второе число: ", &second)) {
            console_printf("\nВвод завершён. Программа остановлена.\n");
            break;
        }

        /*
         * Здесь нет switch по операциям: вызывается функция, адрес которой
         * хранится в выбранной динамической команде.
         */
        status = calculator_command_execute(
            command,
            first,
            second,
            &result
        );

        if (status == CALCULATOR_OK) {
            console_printf("Операция: %s\n", command->name);
            console_printf("Результат: %.10g\n", result);
        } else {
            print_calculation_error(status);
        }
    }

    calculator_menu_destroy(&menu);
    return 0;
}
