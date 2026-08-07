#include "plugin_api.h"

CALCULATOR_PLUGIN_EXPORT CalculatorStatus add(
    double first,
    double second,
    double *result
)
{
    if (result == 0) {
        return CALCULATOR_NULL_RESULT;
    }

    *result = first + second;
    return CALCULATOR_OK;
}
