#include "plugin_loader.h"

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
#define DEFAULT_PLUGIN_DIRECTORY "plugins"

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

static void print_menu(const CalculatorMenu *menu)
{
    size_t index;

    console_printf("\n");
    console_printf("========================================\n");
    console_printf("   КАЛЬКУЛЯТОР С ДИНАМИЧЕСКИМИ DLL/SO   \n");
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

static void print_loader_error(PluginLoaderStatus status)
{
    if (status == PLUGIN_LOADER_DIRECTORY_ERROR) {
        console_printf("Ошибка: не удалось открыть каталог библиотек.\n");
    } else if (status == PLUGIN_LOADER_OUT_OF_MEMORY) {
        console_printf("Ошибка: недостаточно памяти при загрузке библиотек.\n");
    } else {
        console_printf("Ошибка загрузчика динамических библиотек.\n");
    }
}

int main(int argc, char *argv[])
{
    const char *plugin_directory;
    PluginRegistry registry;
    PluginLoaderStatus loader_status;
    int choice;

    configure_console();

    if (argc > 2) {
        console_printf("Использование: %s [каталог_библиотек]\n", argv[0]);
        return 1;
    }

    plugin_directory = argc == 2 ? argv[1] : DEFAULT_PLUGIN_DIRECTORY;

    plugin_registry_init(&registry);
    loader_status = plugin_registry_load_directory(&registry, plugin_directory);

    if (loader_status != PLUGIN_LOADER_OK) {
        print_loader_error(loader_status);
        console_printf("Каталог: %s\n", plugin_directory);
        plugin_registry_destroy(&registry);
        return 1;
    }

    console_printf("Каталог библиотек: %s\n", plugin_directory);
    console_printf("Загружено операций: %zu\n", registry.count);

    if (registry.skipped_count > 0U) {
        console_printf(
            "Пропущено неподходящих библиотек: %zu\n",
            registry.skipped_count
        );
    }

    if (registry.count == 0U) {
        console_printf(
            "В каталоге нет подходящих динамических библиотек.\n"
        );
        plugin_registry_destroy(&registry);
        return 1;
    }

    for (;;) {
        const CalculatorCommand *command;
        double first;
        double second;
        double result;
        CalculatorStatus status;

        print_menu(&registry.menu);

        if (!read_int("Выберите действие: ", &choice)) {
            console_printf("\nВвод завершён. Программа остановлена.\n");
            break;
        }

        if (choice == 0) {
            console_printf("Работа калькулятора завершена.\n");
            break;
        }

        command = calculator_menu_find(&registry.menu, choice);

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
         * Никакого switch по арифметическим операциям здесь нет.
         * Вызывается указатель на функцию, полученный из DLL/SO во время
         * запуска программы.
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

    plugin_registry_destroy(&registry);
    return 0;
}
