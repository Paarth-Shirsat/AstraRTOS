#ifndef OVERFLOW_H
#define OVERFLOW_H

#include <stdint.h>
#include "task.h"

#define SCB_HFSR (*(volatile uint32_t *)0xE000ED2C)
#define SCB_CFSR (*(volatile uint32_t *)0xE000ED28)
#define SCB_MMFSR (*(volatile uint32_t *)0xE000ED28)

void apply_guard_region(os_tcb_t *task);
void print_task_logs(void);
void MemManage_Handler(void);
void HardFault_Handler(void);

#endif
