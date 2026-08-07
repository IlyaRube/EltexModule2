#include "tests.h"
#include "priority_queue.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define EXPECT_TRUE(condition)                                                   \
    do {                                                                         \
        tests_run++;                                                             \
        if (condition) {                                                         \
            tests_passed++;                                                      \
            printf("[PASS] %s\n", #condition);                                  \
        } else {                                                                 \
            printf("[FAIL] %s (строка %d)\n", #condition, __LINE__);             \
        }                                                                        \
    } while (0)

static void test_empty_queue(void)
{
    PriorityQueue queue;
    Message message;

    printf("\nТест: пустая очередь\n");
    pq_init(&queue);

    EXPECT_TRUE(pq_is_empty(&queue));
    EXPECT_TRUE(pq_size(&queue) == 0);
    EXPECT_TRUE(!pq_pop_front(&queue, &message));
    EXPECT_TRUE(!pq_pop_priority(&queue, 100, &message));
    EXPECT_TRUE(!pq_pop_at_least(&queue, 100, &message));

    pq_clear(&queue);
}

static void test_priority_order(void)
{
    PriorityQueue queue;
    Message message;

    printf("\nТест: порядок по приоритету\n");
    pq_init(&queue);

    EXPECT_TRUE(pq_enqueue(&queue, 10, "low"));
    EXPECT_TRUE(pq_enqueue(&queue, 200, "high"));
    EXPECT_TRUE(pq_enqueue(&queue, 100, "middle"));

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(message.priority == 200);
    EXPECT_TRUE(strcmp(message.text, "high") == 0);

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(message.priority == 100);

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(message.priority == 10);
    EXPECT_TRUE(pq_is_empty(&queue));

    pq_clear(&queue);
}

static void test_fifo_for_equal_priority(void)
{
    PriorityQueue queue;
    Message message;

    printf("\nТест: FIFO для одинакового приоритета\n");
    pq_init(&queue);

    EXPECT_TRUE(pq_enqueue(&queue, 150, "first"));
    EXPECT_TRUE(pq_enqueue(&queue, 150, "second"));
    EXPECT_TRUE(pq_enqueue(&queue, 150, "third"));

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(strcmp(message.text, "first") == 0);

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(strcmp(message.text, "second") == 0);

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(strcmp(message.text, "third") == 0);

    pq_clear(&queue);
}

static void test_pop_exact_priority(void)
{
    PriorityQueue queue;
    Message message;

    printf("\nТест: извлечение с указанным приоритетом\n");
    pq_init(&queue);

    pq_enqueue(&queue, 220, "p220");
    pq_enqueue(&queue, 150, "p150-first");
    pq_enqueue(&queue, 150, "p150-second");
    pq_enqueue(&queue, 20, "p20");

    EXPECT_TRUE(pq_pop_priority(&queue, 150, &message));
    EXPECT_TRUE(message.priority == 150);
    EXPECT_TRUE(strcmp(message.text, "p150-first") == 0);
    EXPECT_TRUE(pq_size(&queue) == 3);

    EXPECT_TRUE(!pq_pop_priority(&queue, 100, &message));
    EXPECT_TRUE(pq_size(&queue) == 3);

    pq_clear(&queue);
}

static void test_pop_at_least(void)
{
    PriorityQueue queue;
    Message message;

    printf("\nТест: извлечение с приоритетом не ниже заданного\n");
    pq_init(&queue);

    pq_enqueue(&queue, 250, "critical");
    pq_enqueue(&queue, 180, "important");
    pq_enqueue(&queue, 40, "ordinary");

    EXPECT_TRUE(pq_pop_at_least(&queue, 200, &message));
    EXPECT_TRUE(message.priority == 250);
    EXPECT_TRUE(strcmp(message.text, "critical") == 0);

    EXPECT_TRUE(!pq_pop_at_least(&queue, 200, &message));
    EXPECT_TRUE(pq_pop_at_least(&queue, 100, &message));
    EXPECT_TRUE(message.priority == 180);

    pq_clear(&queue);
}

static void test_boundaries(void)
{
    PriorityQueue queue;
    Message message;

    printf("\nТест: граничные приоритеты 0 и 255\n");
    pq_init(&queue);

    EXPECT_TRUE(pq_enqueue(&queue, 0, "min"));
    EXPECT_TRUE(pq_enqueue(&queue, 255, "max"));

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(message.priority == 255);

    EXPECT_TRUE(pq_pop_front(&queue, &message));
    EXPECT_TRUE(message.priority == 0);

    pq_clear(&queue);
}

int run_all_tests(void)
{
    tests_run = 0;
    tests_passed = 0;

    printf("================ АВТОТЕСТЫ ================\n");

    test_empty_queue();
    test_priority_order();
    test_fifo_for_equal_priority();
    test_pop_exact_priority();
    test_pop_at_least();
    test_boundaries();

    printf("\n============================================\n");
    printf("Пройдено: %d из %d\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("Результат: ВСЕ ТЕСТЫ ПРОЙДЕНЫ\n");
        return 1;
    }

    printf("Результат: ЕСТЬ ОШИБКИ\n");
    return 0;
}

#ifdef TEST_MAIN
int main(void)
{
    return run_all_tests() ? 0 : 1;
}
#endif
