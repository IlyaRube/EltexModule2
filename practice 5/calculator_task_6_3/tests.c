#include "plugin_loader.h"

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

static CalculatorStatus dummy_add(
    double first,
    double second,
    double *result
)
{
    if (result == NULL) {
        return CALCULATOR_NULL_RESULT;
    }

    *result = first + second;
    return CALCULATOR_OK;
}

static CalculatorStatus dummy_multiply(
    double first,
    double second,
    double *result
)
{
    if (result == NULL) {
        return CALCULATOR_NULL_RESULT;
    }

    *result = first * second;
    return CALCULATOR_OK;
}

static void test_dynamic_menu_core(void)
{
    CalculatorMenu menu;
    CalculatorStatus status;
    const CalculatorCommand *command;
    double result = 0.0;

    calculator_menu_init(&menu);

    EXPECT_TRUE(menu.commands == NULL);
    EXPECT_TRUE(menu.count == 0U);
    EXPECT_TRUE(menu.capacity == 0U);

    status = calculator_menu_add(&menu, 1, "dummy_add", dummy_add);
    EXPECT_TRUE(status == CALCULATOR_OK);

    status = calculator_menu_add(&menu, 2, "dummy_multiply", dummy_multiply);
    EXPECT_TRUE(status == CALCULATOR_OK);

    command = calculator_menu_find(&menu, 2);
    EXPECT_TRUE(command != NULL);
    EXPECT_TRUE(command != NULL && command->operation == dummy_multiply);

    status = calculator_command_execute(command, 6.0, 7.0, &result);
    EXPECT_TRUE(status == CALCULATOR_OK);
    EXPECT_NEAR(42.0, result, 1e-9);

    EXPECT_TRUE(
        calculator_menu_add(&menu, 1, "another", dummy_add) ==
        CALCULATOR_DUPLICATE_COMMAND
    );
    EXPECT_TRUE(
        calculator_menu_add(&menu, 3, "dummy_add", dummy_add) ==
        CALCULATOR_DUPLICATE_COMMAND
    );
    EXPECT_TRUE(calculator_menu_find(&menu, 99) == NULL);
    EXPECT_TRUE(calculator_menu_find_by_name(&menu, "missing") == NULL);
    EXPECT_TRUE(calculator_menu_get(&menu, 99U) == NULL);

    calculator_menu_destroy(&menu);
    EXPECT_TRUE(menu.commands == NULL);
    EXPECT_TRUE(menu.count == 0U);
    EXPECT_TRUE(menu.capacity == 0U);
}

static void check_operation(
    const PluginRegistry *registry,
    const char *name,
    double first,
    double second,
    CalculatorStatus expected_status,
    double expected_result
)
{
    const CalculatorCommand *command;
    CalculatorStatus status;
    double result = 123456.0;

    command = calculator_menu_find_by_name(&registry->menu, name);
    EXPECT_TRUE(command != NULL);

    if (command == NULL) {
        return;
    }

    status = calculator_command_execute(command, first, second, &result);
    EXPECT_TRUE(status == expected_status);

    if (status == CALCULATOR_OK) {
        EXPECT_NEAR(expected_result, result, 1e-9);
    }
}

static void test_loading_real_dynamic_libraries(void)
{
    PluginRegistry registry;
    PluginLoaderStatus status;

    plugin_registry_init(&registry);
    status = plugin_registry_load_directory(&registry, "plugins");

    EXPECT_TRUE(status == PLUGIN_LOADER_OK);
    EXPECT_TRUE(registry.count == 4U);
    EXPECT_TRUE(registry.menu.count == 4U);
    EXPECT_TRUE(registry.skipped_count == 0U);

    check_operation(
        &registry,
        "add",
        8.0,
        2.0,
        CALCULATOR_OK,
        10.0
    );
    check_operation(
        &registry,
        "subtract",
        8.0,
        2.0,
        CALCULATOR_OK,
        6.0
    );
    check_operation(
        &registry,
        "multiply",
        8.0,
        2.0,
        CALCULATOR_OK,
        16.0
    );
    check_operation(
        &registry,
        "divide",
        8.0,
        2.0,
        CALCULATOR_OK,
        4.0
    );
    check_operation(
        &registry,
        "divide",
        8.0,
        0.0,
        CALCULATOR_DIVISION_BY_ZERO,
        0.0
    );

    plugin_registry_destroy(&registry);
    EXPECT_TRUE(registry.plugins == NULL);
    EXPECT_TRUE(registry.count == 0U);
    EXPECT_TRUE(registry.menu.commands == NULL);
}

static void test_missing_directory(void)
{
    PluginRegistry registry;
    PluginLoaderStatus status;

    plugin_registry_init(&registry);
    status = plugin_registry_load_directory(
        &registry,
        "__directory_that_does_not_exist__"
    );

    EXPECT_TRUE(status == PLUGIN_LOADER_DIRECTORY_ERROR);
    EXPECT_TRUE(registry.count == 0U);
    EXPECT_TRUE(registry.menu.count == 0U);

    plugin_registry_destroy(&registry);
}

int main(void)
{
    test_dynamic_menu_core();
    test_loading_real_dynamic_libraries();
    test_missing_directory();

    printf("\nTests passed: %d/%d\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("ALL TESTS PASSED\n");
        return 0;
    }

    printf("SOME TESTS FAILED\n");
    return 1;
}
