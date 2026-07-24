#include "chmod_parser.h"
#include "file_info.h"
#include "permissions.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define EXPECT_TRUE(condition)                                                     \
    do {                                                                           \
        tests_run++;                                                               \
        if (condition) {                                                           \
            tests_passed++;                                                        \
            printf("[PASS] %s\n", #condition);                                    \
        } else {                                                                   \
            printf("[FAIL] %s (%s:%d)\n", #condition, __FILE__, __LINE__);         \
        }                                                                          \
    } while (0)

#define EXPECT_MODE(expected, actual)                                              \
    do {                                                                           \
        mode_t expected_value = (mode_t)(expected);                                \
        mode_t actual_value = (mode_t)(actual);                                    \
        tests_run++;                                                               \
        if (expected_value == actual_value) {                                      \
            tests_passed++;                                                        \
            printf("[PASS] %03o == %03o\n",                                      \
                   (unsigned)expected_value, (unsigned)actual_value);               \
        } else {                                                                   \
            printf("[FAIL] expected %03o, actual %03o (%s:%d)\n",                 \
                   (unsigned)expected_value, (unsigned)actual_value,                \
                   __FILE__, __LINE__);                                             \
        }                                                                          \
    } while (0)

static void test_permissions_parse(void)
{
    mode_t mode;
    char error[128];

    EXPECT_TRUE(permissions_parse("755", &mode, error, sizeof(error)) == 0);
    EXPECT_MODE(0755, mode);

    EXPECT_TRUE(permissions_parse("0750", &mode, error, sizeof(error)) == 0);
    EXPECT_MODE(0750, mode);

    EXPECT_TRUE(permissions_parse("rwxr-xr--", &mode,
                                  error, sizeof(error)) == 0);
    EXPECT_MODE(0754, mode);

    EXPECT_TRUE(permissions_parse("-rw-r-----", &mode,
                                  error, sizeof(error)) == 0);
    EXPECT_MODE(0640, mode);

    EXPECT_TRUE(permissions_parse("888", &mode,
                                  error, sizeof(error)) != 0);
    EXPECT_TRUE(permissions_parse("rwxr-x", &mode,
                                  error, sizeof(error)) != 0);
}

static void test_permissions_format(void)
{
    char symbolic[PERMISSIONS_SYMBOLIC_SIZE];
    char bits[PERMISSIONS_BITS_SIZE];
    char octal[PERMISSIONS_OCTAL_SIZE];

    permissions_to_symbolic(0754, symbolic);
    permissions_to_bits(0754, bits);
    permissions_to_octal(0754, octal);

    EXPECT_TRUE(strcmp(symbolic, "rwxr-xr--") == 0);
    EXPECT_TRUE(strcmp(bits, "111 101 100") == 0);
    EXPECT_TRUE(strcmp(octal, "754") == 0);
}

static void expect_chmod(mode_t initial, const char *command,
                         mode_t expected)
{
    mode_t actual = initial;
    char error[128];
    int status = chmod_apply(initial, command, &actual,
                             error, sizeof(error));

    EXPECT_TRUE(status == 0);
    if (status == 0) {
        EXPECT_MODE(expected, actual);
    }
}

static void test_chmod_commands(void)
{
    mode_t unchanged = 0644;
    mode_t actual = unchanged;
    char error[128];

    expect_chmod(0644, "u+x", 0744);
    expect_chmod(0675, "g-w", 0655);
    expect_chmod(0777, "o=r", 0774);
    expect_chmod(0000, "a+r", 0444);
    expect_chmod(0400, "ug+wx", 0730);
    expect_chmod(0640, "+x", 0751);
    expect_chmod(0755, "u=rw", 0655);
    expect_chmod(0754, "g=u", 0774);
    expect_chmod(0644, "u+r-w", 0444);
    expect_chmod(0666, "u+x,g-w,o=r", 0744);
    expect_chmod(0000, "755", 0755);
    expect_chmod(0000, "rwxr-x---", 0750);
    expect_chmod(0777, "u=,g=rx,o=r", 0054);

    EXPECT_TRUE(chmod_apply(unchanged, "u+", &actual,
                            error, sizeof(error)) != 0);
    EXPECT_MODE(unchanged, actual);

    EXPECT_TRUE(chmod_apply(unchanged, "u+z", &actual,
                            error, sizeof(error)) != 0);
    EXPECT_MODE(unchanged, actual);

    EXPECT_TRUE(chmod_apply(unchanged, "u+x,", &actual,
                            error, sizeof(error)) != 0);
    EXPECT_MODE(unchanged, actual);
}

static void test_file_info(void)
{
    FileInfo info;
    char error[256];

    EXPECT_TRUE(file_info_read("Makefile", &info,
                               error, sizeof(error)) == 0);
    EXPECT_TRUE((info.permissions & ~PERMISSIONS_MASK) == 0);

    EXPECT_TRUE(file_info_read("file_that_does_not_exist_12345", &info,
                               error, sizeof(error)) != 0);
}

int main(void)
{
    test_permissions_parse();
    test_permissions_format();
    test_chmod_commands();
    test_file_info();

    printf("\nTests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
