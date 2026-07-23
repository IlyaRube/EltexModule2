#include "calculator.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_MENU_CAPACITY 4U

static CalculatorStatus validate_result_pointer(double *result)
{
    if (result == NULL) {
        return CALCULATOR_NULL_RESULT;
    }

    return CALCULATOR_OK;
}

static char *copy_string(const char *text)
{
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text);
    copy = malloc(length + 1U);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, text, length + 1U);
    return copy;
}

static CalculatorStatus calculator_menu_reserve(
    CalculatorMenu *menu,
    size_t required_capacity
)
{
    CalculatorCommand *new_commands;
    size_t new_capacity;

    if (menu == NULL) {
        return CALCULATOR_INVALID_ARGUMENT;
    }

    if (required_capacity <= menu->capacity) {
        return CALCULATOR_OK;
    }

    new_capacity = menu->capacity == 0U
        ? INITIAL_MENU_CAPACITY
        : menu->capacity;

    while (new_capacity < required_capacity) {
        new_capacity *= 2U;
    }

    new_commands = realloc(
        menu->commands,
        new_capacity * sizeof(*new_commands)
    );

    if (new_commands == NULL) {
        return CALCULATOR_OUT_OF_MEMORY;
    }

    menu->commands = new_commands;
    menu->capacity = new_capacity;
    return CALCULATOR_OK;
}

CalculatorStatus calculator_add(double first, double second, double *result)
{
    CalculatorStatus status = validate_result_pointer(result);

    if (status != CALCULATOR_OK) {
        return status;
    }

    *result = first + second;
    return CALCULATOR_OK;
}

CalculatorStatus calculator_subtract(double first, double second, double *result)
{
    CalculatorStatus status = validate_result_pointer(result);

    if (status != CALCULATOR_OK) {
        return status;
    }

    *result = first - second;
    return CALCULATOR_OK;
}

CalculatorStatus calculator_multiply(double first, double second, double *result)
{
    CalculatorStatus status = validate_result_pointer(result);

    if (status != CALCULATOR_OK) {
        return status;
    }

    *result = first * second;
    return CALCULATOR_OK;
}

CalculatorStatus calculator_divide(double first, double second, double *result)
{
    CalculatorStatus status = validate_result_pointer(result);

    if (status != CALCULATOR_OK) {
        return status;
    }

    if (second == 0.0) {
        return CALCULATOR_DIVISION_BY_ZERO;
    }

    *result = first / second;
    return CALCULATOR_OK;
}

void calculator_menu_init(CalculatorMenu *menu)
{
    if (menu == NULL) {
        return;
    }

    menu->commands = NULL;
    menu->count = 0U;
    menu->capacity = 0U;
}

void calculator_menu_destroy(CalculatorMenu *menu)
{
    size_t index;

    if (menu == NULL) {
        return;
    }

    for (index = 0U; index < menu->count; index++) {
        free(menu->commands[index].name);
    }

    free(menu->commands);
    menu->commands = NULL;
    menu->count = 0U;
    menu->capacity = 0U;
}

CalculatorStatus calculator_menu_add(
    CalculatorMenu *menu,
    int id,
    const char *name,
    CalculatorOperation operation
)
{
    CalculatorStatus status;
    char *name_copy;

    if (menu == NULL || id <= 0 || name == NULL || name[0] == '\0' ||
        operation == NULL) {
        return CALCULATOR_INVALID_ARGUMENT;
    }

    if (calculator_menu_find(menu, id) != NULL) {
        return CALCULATOR_DUPLICATE_COMMAND;
    }

    status = calculator_menu_reserve(menu, menu->count + 1U);

    if (status != CALCULATOR_OK) {
        return status;
    }

    name_copy = copy_string(name);

    if (name_copy == NULL) {
        return CALCULATOR_OUT_OF_MEMORY;
    }

    menu->commands[menu->count].id = id;
    menu->commands[menu->count].name = name_copy;
    menu->commands[menu->count].operation = operation;
    menu->count++;

    return CALCULATOR_OK;
}

const CalculatorCommand *calculator_menu_find(
    const CalculatorMenu *menu,
    int id
)
{
    size_t index;

    if (menu == NULL) {
        return NULL;
    }

    for (index = 0U; index < menu->count; index++) {
        if (menu->commands[index].id == id) {
            return &menu->commands[index];
        }
    }

    return NULL;
}

const CalculatorCommand *calculator_menu_get(
    const CalculatorMenu *menu,
    size_t index
)
{
    if (menu == NULL || index >= menu->count) {
        return NULL;
    }

    return &menu->commands[index];
}

CalculatorStatus calculator_command_execute(
    const CalculatorCommand *command,
    double first,
    double second,
    double *result
)
{
    if (command == NULL || command->operation == NULL) {
        return CALCULATOR_COMMAND_NOT_FOUND;
    }

    return command->operation(first, second, result);
}
