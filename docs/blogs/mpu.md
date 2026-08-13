# MPU Driver

MPU(Memory Protection Unit) is used to protect various regions and adjust their access permissions.

The **ARM Cortex-M Core** has built-in MPU which has 8 configurable regions which can be used to protect and manage various memory regions.

We are using it to detect stack overflow in our RTOS. We use it to configure a MPU region and place a guard region(32 bytes) below the tasks stack to avoid overflow. The region is then reconfigured at every context switch to protect the current executing task from overflow. 

## Memory Map
These are some of the registers that are used when using the MPU:

* MPU_CTRL(MPU control register) - `0xE000ED94`
* MPU_RBAR(MPU region base address register) - `0xE000ED9C`
* MPU_RASR(MPU region attribute and size register) - `0xE000EDA0`
* SHCSR(System Handler Control and State Register) - `0xE000ED24`

## Control register
This register is used to control the state of MPU.

* Enable the MPU - `MPU_CTRL = (1 << 0)`
* HFNMIENA mask(to keep MPU on during NMI, hardfault, other faults) - `MPU_CTRL = (1 << 1)`
* PRIVDEFENA mask(to let privileged code access unconfigured memory) - `MPU_CTRL = (1 << 2)`

## RBAR
This register is used to select and configure a specific MPU region.
* To select which region to use, you write the region number in the first 4 bits(from 0 to 7).
* Set the VALID bit, to tell the mpu that the region is valid - `MPU_RBAR = (1 << 4)`

Region number must be from 0 <= region_num <= 7.
* Set the (aligned) base address starting from bit 5.

## RASR
This register is used to set the properties of the region.
* Enable the region - `MPU_RASR = (1 << 0)`
* Set the encoded region size in bits 1 to 5.

Region size (in bytes) = `2^(SIZE + 1)`, where SIZE is the 5-bit encoded value written into bits [5:1].  
* SRD(sub-region disable) bits from 8 to 15 are used to disable their respective sub-regions(1/8th of region).  

|Encoded size|Size in bytes|
|---|---|
|00100 (4)| 32(minimum permitted)|
|00101 (5)| 64|
|00110 (6)| 128|
|00111 (7)| 256|
|01000 (8)| 512|
|01001 (9)| 1024|
and so on..

* To set the access permissions by toggling the bits from bit 24 to 26.

|Bits|Access Permissions|
|---|---|
|000| No access|
|011| Read and write(full access)| 
|111| Read only|

* Set the exec_never, bit 28, when 1 blocks code execution from the region; 0 permits it.

## Initialising MPU
```c
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
```
`os_current_task_ptr` is the pointer to the current executing task which is initialised as 0 initially.
* Check if there are any tasks created before.
* Enable the MemManage Fault as per your choice by toggling the 16th bit of SHCSR register.
* Use DMB(Data Memory Barrier) to run all instructions before toggling MPU.
* Enable the MPU by masking the the 0th bit in MPU_CTRL and bit 2 to enable the PRIVDEFENA mask
* Ensure you call DSB(Data Synchronisation Barrier) and ISB(Instruction Synchronisation Barrier) after it. 

DSB blocks the core until all pending memory accesses have completed.
ISB flushes the pipeline so that any instructions already fetched are discarded and re-fetched, ensuring they see the updated MPU configuration.

## Configuring a region
```c
void os_mpu_configure_region(uint32_t region_num, uint32_t stack_base, uint32_t encoded_region_size,
                             uint32_t access_perm, uint32_t exec_never) {
    MPU_RBAR = (stack_base & ~0x1F) | (1 << 4) | // region number valid
               region_num;

    MPU_RASR = (1 << 0) |                             // enable the region
               (encoded_region_size << 1) | // log2(size in bytes) - 1
               (access_perm << 24) | (exec_never << 28);
}
```
* Put the stack base address into the RBAR register and make sure it is aligned to the region size (since our guard region is fixed at 32 bytes, stack_base only needs 32-byte alignment here). 
* Add the encoded region size(`log2(region size in bytes) - 1`)
* Set the access permissions and the exec_never bits in the RASR too.

### Why 32-byte alignment?
The MPU_RBAR register ignores the lower 5 bits of the address you write in. So the base address must already be a multiple of 32 bytes otherwise those bits would silently get masked off pointing the region somewhere other than where you intended.

## Implementation 
```c
void apply_guard_region(os_tcb_t *task) {
    os_mpu_configure_region(1, (uint32_t)(task->stack_base), 0x04, 0, 1);
}
```
* We give the task's stack base address.
* Set the guard region 32 bytes long.
* Set the region as no access 
* Set the exec_never to 1 to not execute any code from this region.


We call the `apply_guard_region()` at every context switch to reconfigure the region to the task's stack.

It is called in the PendSV handler to make sure it runs at every context switch. `apply_guard_region()` only has an effect if `os_mpu_init()` was called before `os_start()`.

When the cpu tries to read/write inside the guard region it causes a MemManage fault. The fault handler then prints all the logs about which task caused it and hangs all the tasks.

## Trade-offs
This approach is deliberately lightweight and costs only one MPU region and adds negligible overhead when nothing goes wrong but it comes with some trade-offs.

* Because the guard is only 32 bytes, a single write that jumps past it entirely (e.g. a large stack-allocated array, or a big local buffer) can corrupt memory beyond the guard without ever touching it, so the overflow goes undetected.
* We have to reconfigure it at every context switch which takes up a few cycles.
* As the guard region is 32 bytes in size, all the task stacks have to be aligned by 32 bytes which wastes a lot of memory.
* Since only one MPU region is reused for the guard, only the currently scheduled task is protected. If a suspended task's stack overflows while it isn't running, that corruption won't be caught until it's scheduled back in and by then the damage may already have spread.

## Example
```c
int main(void) {
    system_init();  
    os_mpu_init();  // Call this to enable the mpu for stack overflow
    os_heap_init();
    os_start();

    return 0;
}
```
Call `os_mpu_init()` before the `os_start()` to make sure the mpu is enabled to detect stack overflow and set the guard region across the task's stack.


