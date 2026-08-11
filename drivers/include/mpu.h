#ifndef MPU_H
#define MPU_H

#include <stdint.h>

#define SHCSR (*(volatile uint32_t *)0xE000ED24)
#define MPU_CTRL (*(volatile uint32_t *)0xE000ED94)
#define MPU_RBAR (*(volatile uint32_t *)0xE000ED9C)
#define MPU_RASR (*(volatile uint32_t *)0xE000EDA0)
extern uint32_t mpu_status;

void os_mpu_init(void);

void os_mpu_configure_region(uint8_t region_num, uint32_t stack_base, uint8_t encoded_region_size,
                             uint8_t access_perm, uint8_t exec_never);

#endif
