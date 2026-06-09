#ifndef H_STATS
#define H_STATS
#include <stdint.h>
typedef struct {
    int task_num;
    uint32_t run_count;
    uint32_t stack_high_water;
    uint32_t stack_size;
}os_task_stats_t;
int os_task_get_stats(int task_num, os_task_stats_t *stats);
#endif
