#include "ipv4.h"
#include "simulation.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define EXPECT_TRUE(condition)                                                 \
    do {                                                                       \
        ++tests_run;                                                           \
        if (condition) {                                                       \
            ++tests_passed;                                                    \
            printf("[PASS] %s\n", #condition);                                 \
        } else {                                                               \
            printf("[FAIL] %s (%s:%d)\n", #condition, __FILE__, __LINE__);      \
        }                                                                      \
    } while (0)

#define EXPECT_U32(expected, actual)                                           \
    do {                                                                       \
        const uint32_t expected_value = (expected);                            \
        const uint32_t actual_value = (actual);                                \
        ++tests_run;                                                           \
        if (expected_value == actual_value) {                                  \
            ++tests_passed;                                                    \
            printf("[PASS] 0x%08X == 0x%08X\n",                                \
                   (unsigned int)expected_value,                               \
                   (unsigned int)actual_value);                                \
        } else {                                                               \
            printf("[FAIL] expected 0x%08X, actual 0x%08X (%s:%d)\n",           \
                   (unsigned int)expected_value,                               \
                   (unsigned int)actual_value,                                 \
                   __FILE__,                                                   \
                   __LINE__);                                                  \
        }                                                                      \
    } while (0)

#define EXPECT_SIZE(expected, actual)                                          \
    do {                                                                       \
        const size_t expected_value = (expected);                              \
        const size_t actual_value = (actual);                                  \
        ++tests_run;                                                           \
        if (expected_value == actual_value) {                                  \
            ++tests_passed;                                                    \
            printf("[PASS] %zu == %zu\n", expected_value, actual_value);        \
        } else {                                                               \
            printf("[FAIL] expected %zu, actual %zu (%s:%d)\n",                 \
                   expected_value,                                             \
                   actual_value,                                               \
                   __FILE__,                                                   \
                   __LINE__);                                                  \
        }                                                                      \
    } while (0)

typedef struct ArrayGeneratorContext {
    const uint32_t *values;
    size_t index;
} ArrayGeneratorContext;

typedef struct ObserverContext {
    size_t calls;
    size_t local_calls;
    size_t external_calls;
    size_t last_packet_number;
    uint32_t last_destination;
} ObserverContext;

static uint32_t array_generator(void *context)
{
    ArrayGeneratorContext *generator = context;
    const uint32_t result = generator->values[generator->index];

    ++generator->index;
    return result;
}

static void counting_observer(size_t packet_number,
                              uint32_t destination,
                              int is_local,
                              void *context)
{
    ObserverContext *observer = context;

    ++observer->calls;
    observer->last_packet_number = packet_number;
    observer->last_destination = destination;

    if (is_local != 0) {
        ++observer->local_calls;
    } else {
        ++observer->external_calls;
    }
}

static uint32_t parse_or_zero(const char *text)
{
    uint32_t address = 0U;

    (void)ipv4_parse(text, &address);
    return address;
}

static void test_ipv4_parse(void)
{
    uint32_t address = 0U;

    EXPECT_TRUE(ipv4_parse("0.0.0.0", &address) == 0);
    EXPECT_U32(0x00000000U, address);

    EXPECT_TRUE(ipv4_parse("255.255.255.255", &address) == 0);
    EXPECT_U32(0xFFFFFFFFU, address);

    EXPECT_TRUE(ipv4_parse("192.168.1.10", &address) == 0);
    EXPECT_U32(0xC0A8010AU, address);

    EXPECT_TRUE(ipv4_parse("10.20.30.40", &address) == 0);
    EXPECT_U32(0x0A141E28U, address);

    EXPECT_TRUE(ipv4_parse("256.1.1.1", &address) != 0);
    EXPECT_TRUE(ipv4_parse("192.168.1", &address) != 0);
    EXPECT_TRUE(ipv4_parse("192.168.1.1.5", &address) != 0);
    EXPECT_TRUE(ipv4_parse("192.168.1.-1", &address) != 0);
    EXPECT_TRUE(ipv4_parse("192.168.one.1", &address) != 0);
    EXPECT_TRUE(ipv4_parse("", &address) != 0);
    EXPECT_TRUE(ipv4_parse(NULL, &address) != 0);
    EXPECT_TRUE(ipv4_parse("127.0.0.1", NULL) != 0);
}

static void test_ipv4_format(void)
{
    char text[IPV4_STRING_SIZE];

    ipv4_format(0x00000000U, text);
    EXPECT_TRUE(strcmp(text, "0.0.0.0") == 0);

    ipv4_format(0xFFFFFFFFU, text);
    EXPECT_TRUE(strcmp(text, "255.255.255.255") == 0);

    ipv4_format(0xC0A8010AU, text);
    EXPECT_TRUE(strcmp(text, "192.168.1.10") == 0);

    ipv4_format(0x0A141E28U, text);
    EXPECT_TRUE(strcmp(text, "10.20.30.40") == 0);
}

static void test_masks(void)
{
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("0.0.0.0")) != 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.0.0.0")) != 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.255.0.0")) != 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.255.255.0")) != 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.255.255.252")) != 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.255.255.255")) != 0);

    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.0.255.0")) == 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.255.0.255")) == 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("255.255.255.1")) == 0);
    EXPECT_TRUE(ipv4_mask_is_valid(parse_or_zero("127.255.255.255")) == 0);

    EXPECT_TRUE(ipv4_mask_prefix_length(parse_or_zero("0.0.0.0")) == 0U);
    EXPECT_TRUE(ipv4_mask_prefix_length(parse_or_zero("255.0.0.0")) == 8U);
    EXPECT_TRUE(ipv4_mask_prefix_length(parse_or_zero("255.255.0.0")) == 16U);
    EXPECT_TRUE(ipv4_mask_prefix_length(parse_or_zero("255.255.255.0")) == 24U);
    EXPECT_TRUE(ipv4_mask_prefix_length(parse_or_zero("255.255.255.252")) == 30U);
    EXPECT_TRUE(ipv4_mask_prefix_length(parse_or_zero("255.255.255.255")) == 32U);
}

static void test_subnet_membership(void)
{
    const uint32_t mask_24 = parse_or_zero("255.255.255.0");
    const uint32_t mask_16 = parse_or_zero("255.255.0.0");
    const uint32_t gateway = parse_or_zero("192.168.1.1");

    EXPECT_TRUE(ipv4_same_subnet(
                    gateway,
                    parse_or_zero("192.168.1.10"),
                    mask_24) != 0);

    EXPECT_TRUE(ipv4_same_subnet(
                    gateway,
                    parse_or_zero("192.168.1.255"),
                    mask_24) != 0);

    EXPECT_TRUE(ipv4_same_subnet(
                    gateway,
                    parse_or_zero("192.168.2.1"),
                    mask_24) == 0);

    EXPECT_TRUE(ipv4_same_subnet(
                    gateway,
                    parse_or_zero("192.168.200.10"),
                    mask_16) != 0);

    EXPECT_TRUE(ipv4_same_subnet(
                    gateway,
                    parse_or_zero("192.169.1.1"),
                    mask_16) == 0);
}

static void test_simulation(void)
{
    const uint32_t destinations[] = {
        0xC0A8010AU, /* 192.168.1.10  - local */
        0xC0A80201U, /* 192.168.2.1   - external */
        0xC0A801FFU, /* 192.168.1.255 - local */
        0x0A000001U  /* 10.0.0.1      - external */
    };

    ArrayGeneratorContext generator = {
        destinations,
        0U
    };

    ObserverContext observer = {
        0U,
        0U,
        0U,
        0U,
        0U
    };

    PacketStatistics statistics = {
        0U,
        0U,
        0U
    };

    const int status = simulation_run(
        parse_or_zero("192.168.1.1"),
        parse_or_zero("255.255.255.0"),
        4U,
        array_generator,
        &generator,
        counting_observer,
        &observer,
        &statistics);

    EXPECT_TRUE(status == 0);
    EXPECT_SIZE(4U, statistics.total);
    EXPECT_SIZE(2U, statistics.local);
    EXPECT_SIZE(2U, statistics.external);
    EXPECT_SIZE(4U, generator.index);

    EXPECT_SIZE(4U, observer.calls);
    EXPECT_SIZE(2U, observer.local_calls);
    EXPECT_SIZE(2U, observer.external_calls);
    EXPECT_SIZE(4U, observer.last_packet_number);
    EXPECT_U32(0x0A000001U, observer.last_destination);

    EXPECT_TRUE(simulation_run(
                    0U,
                    0U,
                    0U,
                    array_generator,
                    &generator,
                    NULL,
                    NULL,
                    &statistics) != 0);

    EXPECT_TRUE(simulation_run(
                    0U,
                    0U,
                    1U,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &statistics) != 0);

    EXPECT_TRUE(simulation_run(
                    0U,
                    0U,
                    1U,
                    array_generator,
                    &generator,
                    NULL,
                    NULL,
                    NULL) != 0);
}

int main(void)
{
    test_ipv4_parse();
    test_ipv4_format();
    test_masks();
    test_subnet_membership();
    test_simulation();

    printf("\nTests: %d/%d passed\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}
