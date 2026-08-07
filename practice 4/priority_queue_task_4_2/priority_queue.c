#include "priority_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_text(char destination[PQ_TEXT_SIZE], const char *source)
{
    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    snprintf(destination, PQ_TEXT_SIZE, "%s", source);
}

static int remove_node(PriorityQueue *queue,
                       QueueNode *previous,
                       QueueNode *node,
                       Message *out_message)
{
    if (queue == NULL || node == NULL) {
        return 0;
    }

    if (out_message != NULL) {
        *out_message = node->message;
    }

    if (previous == NULL) {
        queue->head = node->next;
    } else {
        previous->next = node->next;
    }

    if (queue->tail == node) {
        queue->tail = previous;
    }

    free(node);
    queue->size--;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    return 1;
}

void pq_init(PriorityQueue *queue)
{
    if (queue == NULL) {
        return;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->next_id = 1;
}

int pq_enqueue(PriorityQueue *queue, uint8_t priority, const char *text)
{
    QueueNode *new_node;
    QueueNode *current;

    if (queue == NULL) {
        return 0;
    }

    new_node = malloc(sizeof(*new_node));
    if (new_node == NULL) {
        return 0;
    }

    new_node->message.id = queue->next_id++;
    new_node->message.priority = priority;
    copy_text(new_node->message.text, text);
    new_node->next = NULL;

    /*
     * Очередь хранится по убыванию приоритета:
     * 255 — самый высокий, 0 — самый низкий.
     * Сообщения с одинаковым приоритетом сохраняют FIFO-порядок.
     */
    if (queue->head == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
        queue->size++;
        return 1;
    }

    if (priority > queue->head->message.priority) {
        new_node->next = queue->head;
        queue->head = new_node;
        queue->size++;
        return 1;
    }

    current = queue->head;
    while (current->next != NULL &&
           current->next->message.priority >= priority) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;

    if (new_node->next == NULL) {
        queue->tail = new_node;
    }

    queue->size++;
    return 1;
}

int pq_pop_front(PriorityQueue *queue, Message *out_message)
{
    if (queue == NULL || queue->head == NULL) {
        return 0;
    }

    return remove_node(queue, NULL, queue->head, out_message);
}

int pq_pop_priority(PriorityQueue *queue,
                    uint8_t priority,
                    Message *out_message)
{
    QueueNode *previous = NULL;
    QueueNode *current;

    if (queue == NULL) {
        return 0;
    }

    current = queue->head;

    while (current != NULL) {
        if (current->message.priority == priority) {
            return remove_node(queue, previous, current, out_message);
        }

        /* Очередь отсортирована, дальше приоритеты будут только меньше. */
        if (current->message.priority < priority) {
            return 0;
        }

        previous = current;
        current = current->next;
    }

    return 0;
}

int pq_pop_at_least(PriorityQueue *queue,
                    uint8_t min_priority,
                    Message *out_message)
{
    if (queue == NULL || queue->head == NULL) {
        return 0;
    }

    /*
     * Голова содержит самый высокий приоритет.
     * Если даже он меньше порога, подходящих элементов нет.
     */
    if (queue->head->message.priority < min_priority) {
        return 0;
    }

    return pq_pop_front(queue, out_message);
}

void pq_print(const PriorityQueue *queue)
{
    const QueueNode *current;

    if (queue == NULL || queue->head == NULL) {
        printf("Очередь пуста.\n");
        return;
    }

    printf("\n%-6s %-10s %s\n", "ID", "Приоритет", "Сообщение");
    printf("-----------------------------------------------\n");

    current = queue->head;
    while (current != NULL) {
        printf("%-6lu %-10u %s\n",
               current->message.id,
               (unsigned int)current->message.priority,
               current->message.text);
        current = current->next;
    }

    printf("Всего элементов: %zu\n", queue->size);
}

size_t pq_size(const PriorityQueue *queue)
{
    return queue == NULL ? 0 : queue->size;
}

int pq_is_empty(const PriorityQueue *queue)
{
    return queue == NULL || queue->size == 0;
}

void pq_clear(PriorityQueue *queue)
{
    QueueNode *current;

    if (queue == NULL) {
        return;
    }

    current = queue->head;
    while (current != NULL) {
        QueueNode *next = current->next;
        free(current);
        current = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}
