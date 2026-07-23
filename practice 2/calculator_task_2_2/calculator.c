#include "calculator.h"

double calculator_add(double a, double b)
{
    return a + b;
}

double calculator_subtract(double a, double b)
{
    return a - b;
}

double calculator_multiply(double a, double b)
{
    return a * b;
}

CalculatorStatus calculator_divide(double a, double b, double *result)
{
    if (result == 0) {
        return CALCULATOR_NULL_RESULT;
    }

    if (b == 0.0) {
        return CALCULATOR_DIVISION_BY_ZERO;
    }

    *result = a / b;
    return CALCULATOR_OK;
}
