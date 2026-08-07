#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stddef.h>
#include <stdint.h>

#define PQ_TEXT_SIZE 128

typedef struct {
    unsigned long id;
    uint8_t priority;
    char text[PQ_TEXT_SIZE];
} Message;

typedef struct QueueNode {
    Message message;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *head;
    QueueNode *tail;
    size_t size;
    unsigned long next_id;
} PriorityQueue;

void pq_init(PriorityQueue *queue);
int pq_enqueue(PriorityQueue *queue, uint8_t priority, const char *text);
int pq_pop_front(PriorityQueue *queue, Message *out_message);
int pq_pop_priority(PriorityQueue *queue, uint8_t priority, Message *out_message);
int pq_pop_at_least(PriorityQueue *queue, uint8_t min_priority, Message *out_message);
void pq_print(const PriorityQueue *queue);
size_t pq_size(const PriorityQueue *queue);
int pq_is_empty(const PriorityQueue *queue);
void pq_clear(PriorityQueue *queue);

#endif
