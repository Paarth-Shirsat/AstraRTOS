#include "mpu.h"
#include "task.h"

uint32_t mpu_status = 0;

void os_mpu_init() {
    if(os_current_task_ptr != 0)
        return;
    SHCSR |= (1 << 16);    // enable Mem manage fault
    __asm volatile("dmb"); // data memory barrier
    MPU_CTRL = 0;          // disable the MPU

    MPU_CTRL = (1 << 0) | (1 << 2);
    __asm volatile("dsb"); // data synchronisation barrier
    __asm volatile("isb"); // instruction synchronisation barrier
    mpu_status = 1;
}

void os_mpu_configure_region(uint8_t region_num, uint32_t stack_base, uint8_t encoded_region_size,
                             uint8_t access_perm, uint8_t exec_never) {
    if(region_num > 7)
        return;
    MPU_RBAR = (stack_base & ~0x1F) | (1 << 4) | // region number valid
               region_num;

    MPU_RASR = (1 << 0) |                   // enable the region
               (encoded_region_size << 1) | // log2(size in bytes) - 1
               (access_perm << 24) | (exec_never << 28);
}
