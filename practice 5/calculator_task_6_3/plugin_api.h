#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#ifdef _WIN32
#define CALCULATOR_PLUGIN_EXPORT __declspec(dllexport)
#else
#define CALCULATOR_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

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
 * Единый интерфейс всех подключаемых арифметических функций.
 * Имя экспортируемой функции должно совпадать с именем файла библиотеки:
 *   add.dll / add.so -> функция add
 *   divide.dll / divide.so -> функция divide
 */
typedef CalculatorStatus (*CalculatorOperation)(
    double first,
    double second,
    double *result
);

#endif
