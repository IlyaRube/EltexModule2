#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stddef.h>

typedef enum CalculatorStatus {
    CALCULATOR_OK = 0,
    CALCULATOR_DIVISION_BY_ZERO,
    CALCULATOR_NULL_RESULT,
    CALCULATOR_INVALID_ARGUMENT,
    CALCULATOR_OUT_OF_MEMORY,
    CALCULATOR_DUPLICATE_COMMAND,
    CALCULATOR_COMMAND_NOT_FOUND
} CalculatorStatus;

/*
 * Единый тип указателя на арифметическую функцию.
 * Любая команда принимает два числа, записывает результат по указателю
 * и возвращает код состояния.
 */
typedef CalculatorStatus (*CalculatorOperation)(
    double first,
    double second,
    double *result
);

typedef struct CalculatorCommand {
    int id;
    char *name;
    CalculatorOperation operation;
} CalculatorCommand;

typedef struct CalculatorMenu {
    CalculatorCommand *commands;
    size_t count;
    size_t capacity;
} CalculatorMenu;

CalculatorStatus calculator_add(double first, double second, double *result);
CalculatorStatus calculator_subtract(double first, double second, double *result);
CalculatorStatus calculator_multiply(double first, double second, double *result);
CalculatorStatus calculator_divide(double first, double second, double *result);

void calculator_menu_init(CalculatorMenu *menu);
void calculator_menu_destroy(CalculatorMenu *menu);

CalculatorStatus calculator_menu_add(
    CalculatorMenu *menu,
    int id,
    const char *name,
    CalculatorOperation operation
);

const CalculatorCommand *calculator_menu_find(
    const CalculatorMenu *menu,
    int id
);

const CalculatorCommand *calculator_menu_get(
    const CalculatorMenu *menu,
    size_t index
);

CalculatorStatus calculator_command_execute(
    const CalculatorCommand *command,
    double first,
    double second,
    double *result
);

#endif
