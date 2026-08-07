#include "plugin_api.h"

CALCULATOR_PLUGIN_EXPORT CalculatorStatus divide(
    double first,
    double second,
    double *result
)
{
    if (result == 0) {
        return CALCULATOR_NULL_RESULT;
    }

    if (second == 0.0) {
        return CALCULATOR_DIVISION_BY_ZERO;
    }

    *result = first / second;
    return CALCULATOR_OK;
}
