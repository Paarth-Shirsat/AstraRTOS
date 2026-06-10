#include "stats.h"
#include "task.h"
extern uint32_t os_get_task_count();
int os_task_get_stats(int task_num, os_task_stats_t *stats){
    if(task_num < 0 || (uint32_t)task_num >= os_get_task_count()){
        return -1;
    }
    stats->task_num = task_num;
    stats->run_count = os_tasks[task_num]->run_count;
    stats->stack_high_water = os_tasks[task_num]->stack_high_water;
    stats->stack_size = os_tasks[task_num]->stack_size;
    return 0;
}
