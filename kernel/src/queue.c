#include "queue.h"
#include "task.h"
#include "heap.h"
#include "uart.h"

extern os_tcb_t *os_tasks[];
extern os_tcb_t *os_current_task_ptr;
extern uint32_t os_get_task_count(void);

//Helper Function
static void queue_memcpy(uint8_t *dst, const uint8_t *src, uint32_t n){
    for(uint32_t i = 0; i < n; i++){
        dst[i] = src[i];
    }
}

static int queue_wake_one(os_queue_t *q, os_task_state waiting_state){
    uint32_t best = 0;
    int chosen = -1;
    uint32_t n = os_get_task_count();

    for(uint32_t i = 0; i < n; i++){
        if(os_tasks[i]->state == waiting_state && os_tasks[i]->waiting_for_resource == (void *)q){
            if(chosen == -1 || os_tasks[i]->priority > best){
                best = os_tasks[i]->priority;
                chosen = (int)i;
            }
        }
    }

    if(chosen != -1){
        os_tasks[chosen]->state = TASK_READY;
        os_tasks[chosen]->waiting_for_resource = NULL;
        return 1;
    }

    return 0;
}

void os_queue_init(os_queue_t *q, uint32_t item_size, uint32_t max_count){
    q->head = NULL;
    q->tail = NULL;
    q->item_size = item_size;
    q->count = 0;
    q->max_count = max_count;
}

int os_queue_send(os_queue_t *q, const void *data){
    
    while(1){
        os_queue_node_t *node = (os_queue_node_t *)os_malloc(sizeof(os_queue_node_t) + q->item_size);

        if(node == NULL){
            return -1;
        }

        queue_memcpy(node->data, (const uint8_t *)data, q->item_size);
        node->next = NULL;

        __asm volatile ("cpsid i");

        if(q->max_count == 0 || q->count < q->max_count){
            if(q->tail == NULL){
                q->head = node;
                q->tail = node;
            }
            else{
                q->tail->next = node;
                q->tail = node;
            }
            q->count++;

            int woke = queue_wake_one(q, TASK_QUEUE_RECV_WAITING);

            __asm volatile ("cpsie i");

            if(woke){
                ICSR |= (1U << 28);
            }

            return 0;
        }
        else{
            __asm volatile("cpsie i");

            os_mfree(node);

            __asm volatile ("cpsid i");

            os_current_task_ptr->state = TASK_QUEUE_SEND_WAITING;
            os_current_task_ptr->waiting_for_resource = (void *)q;

            __asm volatile ("cpsie i");

            ICSR |= (1U << 28);
        }
    }
}

int os_queue_receive(os_queue_t *q, void *buffer){
    while(1){
        __asm volatile ("cpsid i");

        if(q->count > 0){
            os_queue_node_t *node = q->head;

            queue_memcpy((uint8_t *)buffer, node->data, q->item_size);

            q->head = node->next;

            if(q->head == NULL){
                q->tail = NULL;
            }

            q->count--;

            int woke = queue_wake_one(q, TASK_QUEUE_SEND_WAITING);

            __asm volatile ("cpsie i");

            os_mfree(node);

            if(woke){
                ICSR |= (1U << 28);
            }

            return 0;
        }
        else{
            os_current_task_ptr->state = TASK_QUEUE_RECV_WAITING;
            os_current_task_ptr->waiting_for_resource = (void *)q;

            __asm volatile ("cpsie i");

            ICSR |= (1U << 28);
        }
    }
}

uint32_t os_queue_count(const os_queue_t *q){
    return q->count;
}