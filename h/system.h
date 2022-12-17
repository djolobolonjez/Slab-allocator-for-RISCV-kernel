#ifndef _system_h
#define _system_h

#include "../lib/hw.h"
#include "defs.h"

#define MEM_ALLOC 0x01
#define MEM_FREE 0x02

#define THREAD_CREATE 0x11
#define THREAD_EXIT 0x12
#define THREAD_DISPATCH 0x13
#define TCB_CREATE 0x14

#define SEM_OPEN 0x21
#define SEM_CLOSE 0x22
#define SEM_WAIT 0x23
#define SEM_SIGNAL 0x24

#define TIME_SLEEP 0x31

#define GETC 0x41
#define PUTC 0x42

#define MAX_BUFF_SIZE 1024
#define BLOCK_SIZE (4096)

#define ECALL asm volatile("ecall");

#define SYSCALL_REG_TWO(a0, a1)({ \
     asm volatile("mv a1, %0" : : "r" (a1)); \
     asm volatile("mv a0, %[num]" : : [num] "r" (a0));  \
})

#define SYSCALL_REG_THREE(a0, a1, a2)({ \
    asm volatile("mv a2, %0" : : "r"(a2)); \
    asm volatile("mv a1, %0" : : "r"(a1)); \
    asm volatile("mv a0, %[num]" : : [num] "r"(a0)); \
})

#define SYSCALL_REG_FOUR(a0, a1, a2, a3)({ \
    asm volatile("mv a1, %0" : : "r"(a1)); \
    asm volatile("mv a2, %0" : : "r"(a2)); \
    asm volatile("mv a3, %0" : : "r"(a3)); \
    asm volatile("mv a0, %[num]" : : [num] "r"(a0)); \
})

#define ABI_REG_THREE(a1, a2, a3)({ \
    asm volatile("mv %[handle], a1" : [handle] "=r"(handle)); \
    asm volatile("mv %[start], a2" : [start] "=r"(start_routine)); \
    asm volatile("mv %[arg], a3" : [arg] "=r"(arg)); \
})

#define LOAD_STACK(stck)({ \
    asm volatile("ld a4, 112(s0)"); \
    asm volatile("mv %[stck], a4" : [stck] "=r"(stck)); \
})

#define W_RET asm volatile("sd a0, 80(s0)");

#define WRITE_READY (*((char*)CONSOLE_STATUS) & CONSOLE_TX_STATUS_BIT)
#define READ_READY (*((char*)CONSOLE_STATUS) & CONSOLE_RX_STATUS_BIT)

#define C_WRITE *(char*)CONSOLE_TX_DATA
#define C_READ *(char*)CONSOLE_RX_DATA


#endif
