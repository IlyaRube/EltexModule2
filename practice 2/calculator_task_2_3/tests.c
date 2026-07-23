#include "calculator.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

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

static void test_arithmetic_functions(void)
{
    double result = 0.0;
    CalculatorStatus status;

    status = calculator_add(2.0, 3.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(5.0, result, 1e-9);

    status = calculator_subtract(2.0, 5.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(-3.0, result, 1e-9);

    status = calculator_multiply(-3.0, 4.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(-12.0, result, 1e-9);

    status = calculator_divide(10.0, 4.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(2.5, result, 1e-9);

    result = 123.0;
    status = calculator_divide(10.0, 0.0, &result);
    EXPECT_TRUE(status == CALCULATOR_DIVISION_BY_ZERO);
    EXPECT_NEAR(123.0, result, 1e-9);

    status = calculator_add(1.0, 2.0, NULL);
    EXPECT_TRUE(status == CALCULATOR_NULL_RESULT);
}

static void test_dynamic_command_menu(void)
{
    CalculatorMenu menu;
    CalculatorStatus status;
    const CalculatorCommand *command;
    double result = 0.0;

    calculator_menu_init(&menu);

    EXPECT_TRUE(menu.commands == NULL);
    EXPECT_TRUE(menu.count == 0U);
    EXPECT_TRUE(menu.capacity == 0U);

    status = calculator_menu_add(&menu, 1, "Add", calculator_add);
    EXPECT_TRUE(status == CALCULATOR_OK);

    status = calculator_menu_add(&menu, 2, "Subtract", calculator_subtract);
    EXPECT_TRUE(status == CALCULATOR_OK);

    status = calculator_menu_add(&menu, 3, "Multiply", calculator_multiply);
    EXPECT_TRUE(status == CALCULATOR_OK);

    status = calculator_menu_add(&menu, 4, "Divide", calculator_divide);
    EXPECT_TRUE(status == CALCULATOR_OK);

    /* Пятая команда проверяет автоматическое расширение массива realloc. */
    status = calculator_menu_add(&menu, 5, "Add again", calculator_add);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_TRUE(menu.count == 5U);
    EXPECT_TRUE(menu.capacity >= menu.count);

    command = calculator_menu_find(&menu, 3);
    EXPECT_TRUE(command != NULL);
    EXPECT_TRUE(command != NULL && command->id == 3);
    EXPECT_TRUE(command != NULL && strcmp(command->name, "Multiply") == 0);
    EXPECT_TRUE(command != NULL && command->operation == calculator_multiply);

    status = calculator_command_execute(command, 6.0, 7.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(42.0, result, 1e-9);

    command = calculator_menu_get(&menu, 0U);
    EXPECT_TRUE(command != NULL);
    EXPECT_TRUE(command != NULL && strcmp(command->name, "Add") == 0);

    EXPECT_TRUE(calculator_menu_get(&menu, 99U) == NULL);
    EXPECT_TRUE(calculator_menu_find(&menu, 99) == NULL);

    status = calculator_menu_add(&menu, 1, "Duplicate", calculator_add);
    EXPECT_TRUE(status == CALCULATOR_DUPLICATE_COMMAND);
    EXPECT_TRUE(menu.count == 5U);

    status = calculator_menu_add(&menu, 0, "Invalid", calculator_add);
    EXPECT_TRUE(status == CALCULATOR_INVALID_ARGUMENT);

    status = calculator_menu_add(&menu, 6, "", calculator_add);
    EXPECT_TRUE(status == CALCULATOR_INVALID_ARGUMENT);

    status = calculator_menu_add(&menu, 6, "No function", NULL);
    EXPECT_TRUE(status == CALCULATOR_INVALID_ARGUMENT);

    status = calculator_command_execute(NULL, 1.0, 2.0, &result);
    EXPECT_TRUE(status == CALCULATOR_COMMAND_NOT_FOUND);

    calculator_menu_destroy(&menu);
    EXPECT_TRUE(menu.commands == NULL);
    EXPECT_TRUE(menu.count == 0U);
    EXPECT_TRUE(menu.capacity == 0U);
}

static void test_all_registered_function_pointers(void)
{
    CalculatorMenu menu;
    const CalculatorCommand *command;
    CalculatorStatus status;
    double result = 0.0;

    calculator_menu_init(&menu);

    EXPECT_TRUE(
        calculator_menu_add(&menu, 1, "Add", calculator_add) == CALCULATOR_OK
    );
    EXPECT_TRUE(
        calculator_menu_add(&menu, 2, "Subtract", calculator_subtract) ==
        CALCULATOR_OK
    );
    EXPECT_TRUE(
        calculator_menu_add(&menu, 3, "Multiply", calculator_multiply) ==
        CALCULATOR_OK
    );
    EXPECT_TRUE(
        calculator_menu_add(&menu, 4, "Divide", calculator_divide) ==
        CALCULATOR_OK
    );

    command = calculator_menu_find(&menu, 1);
    status = calculator_command_execute(command, 8.0, 2.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(10.0, result, 1e-9);

    command = calculator_menu_find(&menu, 2);
    status = calculator_command_execute(command, 8.0, 2.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(6.0, result, 1e-9);

    command = calculator_menu_find(&menu, 3);
    status = calculator_command_execute(command, 8.0, 2.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(16.0, result, 1e-9);

    command = calculator_menu_find(&menu, 4);
    status = calculator_command_execute(command, 8.0, 2.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(4.0, result, 1e-9);

    calculator_menu_destroy(&menu);
}

int main(void)
{
    test_arithmetic_functions();
    test_dynamic_command_menu();
    test_all_registered_function_pointers();

    printf("\nTests passed: %d/%d\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("ALL TESTS PASSED\n");
        return 0;
    }

    printf("SOME TESTS FAILED\n");
    return 1;
}
