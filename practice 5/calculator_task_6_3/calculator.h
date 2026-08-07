#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "plugin_api.h"

#include <stddef.h>

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

const CalculatorCommand *calculator_menu_find_by_name(
    const CalculatorMenu *menu,
    const char *name
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
