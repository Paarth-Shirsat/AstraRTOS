#ifndef H_QUEUE
#define H_QUEUE

#include <stdint.h>
#include <stddef.h>

typedef struct os_queue_node{
    struct os_queue_node *next;
    uint8_t data[];
}os_queue_node_t;

typedef struct{
    os_queue_node_t *head;
    os_queue_node_t *tail;
    uint32_t item_size;
    uint32_t count;
    uint32_t max_count;
}os_queue_t;

void os_queue_init(os_queue_t *q, uint32_t item_size, uint32_t max_count);
int os_queue_send(os_queue_t *q, const void *data);
int os_queue_receive(os_queue_t *q, void *buffer);
uint32_t os_queue_count(const os_queue_t *q);

#endif