#include "calculator.h"

#include <math.h>
#include <stdio.h>

static int tests_run = 0;
static int tests_passed = 0;

#define EXPECT_TRUE(condition)                                                    \
    do {                                                                          \
        tests_run++;                                                              \
        if (condition) {                                                          \
            tests_passed++;                                                       \
            printf("[PASS] %s\n", #condition);                                  \
        } else {                                                                  \
            printf("[FAIL] %s (%s:%d)\n", #condition, __FILE__, __LINE__);       \
        }                                                                         \
    } while (0)

#define EXPECT_NEAR(expected, actual, epsilon)                                    \
    do {                                                                          \
        double expected_value = (expected);                                       \
        double actual_value = (actual);                                           \
        double difference = fabs(expected_value - actual_value);                  \
        tests_run++;                                                              \
        if (difference <= (epsilon)) {                                            \
            tests_passed++;                                                       \
            printf("[PASS] %s ~= %s\n", #expected, #actual);                    \
        } else {                                                                  \
            printf("[FAIL] expected %.12g, got %.12g (%s:%d)\n",                \
                   expected_value, actual_value, __FILE__, __LINE__);             \
        }                                                                         \
    } while (0)

static void test_addition(void)
{
    EXPECT_NEAR(5.0, calculator_add(2.0, 3.0), 1e-9);
    EXPECT_NEAR(-5.0, calculator_add(-2.0, -3.0), 1e-9);
    EXPECT_NEAR(0.0, calculator_add(7.5, -7.5), 1e-9);
}

static void test_subtraction(void)
{
    EXPECT_NEAR(2.0, calculator_subtract(5.0, 3.0), 1e-9);
    EXPECT_NEAR(-8.0, calculator_subtract(-5.0, 3.0), 1e-9);
    EXPECT_NEAR(0.0, calculator_subtract(4.25, 4.25), 1e-9);
}

static void test_multiplication(void)
{
    EXPECT_NEAR(12.0, calculator_multiply(3.0, 4.0), 1e-9);
    EXPECT_NEAR(-12.0, calculator_multiply(-3.0, 4.0), 1e-9);
    EXPECT_NEAR(0.0, calculator_multiply(100.0, 0.0), 1e-9);
}

static void test_division(void)
{
    double result = 0.0;
    CalculatorStatus status;

    status = calculator_divide(10.0, 2.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(5.0, result, 1e-9);

    status = calculator_divide(-9.0, 3.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(-3.0, result, 1e-9);

    result = 123.0;
    status = calculator_divide(10.0, 0.0, &result);
    EXPECT_TRUE(status == CALCULATOR_DIVISION_BY_ZERO);
    EXPECT_NEAR(123.0, result, 1e-9);

    status = calculator_divide(10.0, 2.0, NULL);
    EXPECT_TRUE(status == CALCULATOR_NULL_RESULT);
}

int main(void)
{
    test_addition();
    test_subtraction();
    test_multiplication();
    test_division();

    printf("\nTests passed: %d/%d\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("ALL TESTS PASSED\n");
        return 0;
    }

    printf("SOME TESTS FAILED\n");
    return 1;
}
