#ifndef CALCULATOR_H
#define CALCULATOR_H

typedef enum CalculatorStatus {
    CALCULATOR_OK = 0,
    CALCULATOR_DIVISION_BY_ZERO,
    CALCULATOR_NULL_RESULT
} CalculatorStatus;

double calculator_add(double a, double b);
double calculator_subtract(double a, double b);
double calculator_multiply(double a, double b);
CalculatorStatus calculator_divide(double a, double b, double *result);

#endif
