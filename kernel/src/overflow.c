#include "overflow.h"
#include "mpu.h"
#include "uart.h"

void apply_guard_region(os_tcb_t *task) {
    os_mpu_configure_region(1, (uint32_t)(task->stack_base), 0x04, 0, 1);
}

void uart_send_hex(uint32_t base, uint32_t val) {
    char buf[11] = "0x00000000";
    for(int i = 9; i >= 2; i--) {
        uint8_t nibble = val & 0xF;
        buf[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        val >>= 4;
    }
    uart_send_string(base, buf);
}

void print_task_logs(void) {
    uart_send_string(USART2_BASE, "Faulting task_num: ");
    uart_send_hex(USART2_BASE, os_current_task_ptr->task_num);
    uart_send_string(USART2_BASE, "\r\nstack_base: ");
    uart_send_hex(USART2_BASE, (uint32_t)os_current_task_ptr->stack_base);
    uart_send_string(USART2_BASE, "\r\nstack_size (words): ");
    uart_send_hex(USART2_BASE, os_current_task_ptr->stack_size);
    uart_send_string(USART2_BASE, "\r\nsaved stack_ptr: ");
    uart_send_hex(USART2_BASE, (uint32_t)os_current_task_ptr->stack_ptr);
    uart_send_string(USART2_BASE, "\r\n");
}

void MemManage_Handler(void) {
    uint32_t cfsr = SCB_CFSR;
    uint8_t mmfsr = cfsr & 0xFF;
    uart_send_string(USART2_BASE, "MemManage fault\r\n");
    if(mmfsr & (1 << 4)) {
        uart_send_string(USART2_BASE, "MSTKERR - MPU blocked exception stacking\r\n");
        print_task_logs();
    }
    while(1) {
    }
}

void HardFault_Handler(void) {
    uint32_t hfsr = SCB_HFSR;
    uint32_t cfsr = SCB_CFSR;
    if(hfsr & (1 << 30)) {
        uint8_t mmfsr = cfsr & 0xFF;
        uint8_t bfsr = (cfsr >> 8) & 0xFF;

        if(mmfsr & (1 << 4)) {
            uart_send_string(USART2_BASE, "MSTKERR - MPU blocked exception stacking\r\n");
            print_task_logs();
        }
        else if(bfsr & (1 << 4)) {
            uart_send_string(USART2_BASE, "STKERR - Bus fault during exception stacking\r\n");
            print_task_logs();
        }
        else {
            uart_send_string(USART2_BASE, "FORCED fault, unknown sub-cause\r\n");
        }
        while(1) {
        }
    }
    else {
        uart_send_string(USART2_BASE, "Normal HardFault\r\n");
        while(1) {
        }
    }
}
