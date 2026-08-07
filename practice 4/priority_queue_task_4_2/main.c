#include "priority_queue.h"
#include "tests.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INPUT_SIZE 256

static int read_int(const char *prompt, int min_value, int max_value, int *result)
{
    char buffer[INPUT_SIZE];
    char *end;
    long value;

    for (;;) {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return 0;
        }

        errno = 0;
        value = strtol(buffer, &end, 10);

        if (end == buffer) {
            printf("Ошибка: введите целое число.\n");
            continue;
        }

        while (*end == ' ' || *end == '\t') {
            end++;
        }

        if (*end != '\n' && *end != '\0') {
            printf("Ошибка: после числа обнаружены лишние символы.\n");
            continue;
        }

        if (errno == ERANGE || value < INT_MIN || value > INT_MAX) {
            printf("Ошибка: число вне допустимого диапазона int.\n");
            continue;
        }

        if (value < min_value || value > max_value) {
            printf("Ошибка: допустимый диапазон от %d до %d.\n",
                   min_value,
                   max_value);
            continue;
        }

        *result = (int)value;
        return 1;
    }
}

static int read_text(const char *prompt, char *buffer, size_t size)
{
    size_t length;

    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }

    return 1;
}

static void print_message(const char *title, const Message *message)
{
    printf("\n%s\n", title);
    printf("ID: %lu\n", message->id);
    printf("Приоритет: %u\n", (unsigned int)message->priority);
    printf("Текст: %s\n", message->text);
}

static void add_message(PriorityQueue *queue)
{
    int priority;
    char text[PQ_TEXT_SIZE];

    if (!read_int("Введите приоритет (0..255): ", 0, 255, &priority)) {
        return;
    }

    if (!read_text("Введите текст сообщения: ", text, sizeof(text))) {
        return;
    }

    if (pq_enqueue(queue, (uint8_t)priority, text)) {
        printf("Сообщение добавлено.\n");
    } else {
        printf("Ошибка: не удалось выделить память.\n");
    }
}

static void extract_front(PriorityQueue *queue)
{
    Message message;

    if (pq_pop_front(queue, &message)) {
        print_message("Извлечен первый элемент очереди:", &message);
    } else {
        printf("Очередь пуста.\n");
    }
}

static void extract_exact_priority(PriorityQueue *queue)
{
    int priority;
    Message message;

    if (!read_int("Введите точный приоритет (0..255): ", 0, 255, &priority)) {
        return;
    }

    if (pq_pop_priority(queue, (uint8_t)priority, &message)) {
        print_message("Извлечено сообщение с указанным приоритетом:", &message);
    } else {
        printf("Сообщений с приоритетом %d нет.\n", priority);
    }
}

static void extract_at_least(PriorityQueue *queue)
{
    int min_priority;
    Message message;

    if (!read_int("Введите минимальный приоритет (0..255): ",
                  0,
                  255,
                  &min_priority)) {
        return;
    }

    if (pq_pop_at_least(queue, (uint8_t)min_priority, &message)) {
        print_message("Извлечено сообщение с приоритетом не ниже заданного:",
                      &message);
    } else {
        printf("Сообщений с приоритетом не ниже %d нет.\n", min_priority);
    }
}

static void generate_messages(PriorityQueue *queue)
{
    int count;
    int i;
    char text[PQ_TEXT_SIZE];

    if (!read_int("Сколько сообщений сгенерировать (1..100): ", 1, 100, &count)) {
        return;
    }

    for (i = 0; i < count; i++) {
        int priority = rand() % 256;
        snprintf(text, sizeof(text), "Автосообщение #%d", i + 1);

        if (!pq_enqueue(queue, (uint8_t)priority, text)) {
            printf("Не удалось добавить сообщение #%d.\n", i + 1);
            return;
        }
    }

    printf("Сгенерировано сообщений: %d\n", count);
}

static void demo_simulation(void)
{
    PriorityQueue queue;
    Message message;
    int i;
    const uint8_t priorities[] = {20, 230, 80, 230, 150, 10, 200, 100};
    const size_t count = sizeof(priorities) / sizeof(priorities[0]);
    char text[PQ_TEXT_SIZE];

    pq_init(&queue);

    printf("\n=============== ДЕМОНСТРАЦИЯ ===============\n");
    printf("Генерируем сообщения с заранее разными приоритетами.\n");

    for (i = 0; i < (int)count; i++) {
        snprintf(text, sizeof(text), "Сообщение %d", i + 1);
        pq_enqueue(&queue, priorities[i], text);
    }

    pq_print(&queue);

    if (pq_pop_front(&queue, &message)) {
        print_message("1) Выборка первого элемента:", &message);
    }

    if (pq_pop_priority(&queue, 150, &message)) {
        print_message("2) Выборка с точным приоритетом 150:", &message);
    }

    if (pq_pop_at_least(&queue, 200, &message)) {
        print_message("3) Выборка с приоритетом не ниже 200:", &message);
    }

    printf("\nОстаток очереди после трех выборок:\n");
    pq_print(&queue);

    pq_clear(&queue);
    printf("=============================================\n");
}

static void print_menu(void)
{
    printf("\n========== ОЧЕРЕДЬ С ПРИОРИТЕТОМ ==========\n");
    printf("1. Добавить сообщение\n");
    printf("2. Извлечь первый элемент очереди\n");
    printf("3. Извлечь элемент с указанным приоритетом\n");
    printf("4. Извлечь элемент с приоритетом не ниже заданного\n");
    printf("5. Показать очередь\n");
    printf("6. Сгенерировать случайные сообщения\n");
    printf("7. Запустить демонстрационную выборку\n");
    printf("8. Запустить автотесты\n");
    printf("9. Очистить очередь\n");
    printf("0. Выход\n");
    printf("=============================================\n");
}

int main(void)
{
    PriorityQueue queue;
    int choice;

    pq_init(&queue);
    srand((unsigned int)time(NULL));

    for (;;) {
        print_menu();

        if (!read_int("Выберите пункт: ", 0, 9, &choice)) {
            printf("\nВвод завершен.\n");
            break;
        }

        switch (choice) {
            case 1:
                add_message(&queue);
                break;
            case 2:
                extract_front(&queue);
                break;
            case 3:
                extract_exact_priority(&queue);
                break;
            case 4:
                extract_at_least(&queue);
                break;
            case 5:
                pq_print(&queue);
                break;
            case 6:
                generate_messages(&queue);
                break;
            case 7:
                demo_simulation();
                break;
            case 8:
                run_all_tests();
                break;
            case 9:
                pq_clear(&queue);
                printf("Очередь очищена.\n");
                break;
            case 0:
                pq_clear(&queue);
                printf("Программа завершена.\n");
                return 0;
            default:
                break;
        }
    }

    pq_clear(&queue);
    return 0;
}
