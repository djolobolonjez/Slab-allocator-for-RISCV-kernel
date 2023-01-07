#include "../../h/syscall_c.h"

void* 
mem_alloc(size_t size)
{

    uint64 num = MEM_ALLOC;
    void* addr;

    size_t blockSize = size/MEM_BLOCK_SIZE + (size % MEM_BLOCK_SIZE != 0 ? 1:0);

    SYSCALL_REG_TWO(num, blockSize);

    ECALL

    asm volatile("mv %[res], a0" : [res] "=r" (addr));

    return (char*)addr;

}

int 
mem_free(void* addr)
{

    uint64 num = MEM_FREE;
    int ret;

    SYSCALL_REG_TWO(num, addr);

    ECALL

    asm volatile("mv %[res], a0" : [res] "=r" (ret));

    return ret;
}

int 
thread_create(thread_t* handle, void(*start_routine)(void*), void* arg)
{

    if(handle == nullptr) return -1;

    uint64 num = THREAD_CREATE;
    int ret = 0;
    TCB* res;

    void* stack = nullptr;
    if(start_routine != nullptr) {
        stack = mem_alloc(DEFAULT_STACK_SIZE );
        if (stack == nullptr) return -1;
    }

    asm volatile("mv a4, %[stack]" : : [stack] "r"(stack));

    SYSCALL_REG_FOUR(num, handle, start_routine, arg);

    ECALL

    asm volatile("mv %[res], a0" : [res] "=r"(res));

    if(res == nullptr) ret = -1;

    return ret;
}

int
thread_start(thread_t* handle)
{
    uint64 num = THREAD_START;
    int ret = 0;

    SYSCALL_REG_TWO(num, handle);

    ECALL

    asm volatile ("mv %[ret], a0" : [ret] "=r"(ret));

    return ret;
}

void 
thread_dispatch()
{

    uint64 num = THREAD_DISPATCH;

    asm volatile("mv a0, %[num]" : : [num] "r"(num));

    ECALL
}

int 
thread_exit()
{

    uint64 num = THREAD_EXIT;
    int ret = 0;

    asm volatile("mv a0, %[num]" : : [num] "r"(num));

    ECALL

    asm volatile("mv %[res], a0" : [res] "=r" (ret));

    return ret;
}

int 
sem_open(sem_t* handle, unsigned init)
{

    if(handle == nullptr) return -1;

    KernelSem* res;

    uint64 num = SEM_OPEN;
    volatile int ret = 0;

    SYSCALL_REG_THREE(num, handle, init);

    ECALL

    asm volatile("mv %[res], a0" : [res] "=r"(res));

    if(res == nullptr) ret = -1;

    return ret;
}

int 
sem_close(sem_t handle)
{

    if(handle == nullptr) return -1;

    uint64 num = SEM_CLOSE;
    int ret = 0;

    SYSCALL_REG_TWO(num, handle);

    ECALL

    asm volatile("mv %[ret], a0" : [ret] "=r"(ret));

    return ret;
}

int 
sem_wait(sem_t id)
{

    if(id == nullptr) return -1;
    uint64 num = SEM_WAIT;
    int ret = 0;

    SYSCALL_REG_TWO(num, id);

    ECALL

    asm volatile("mv %[ret], a0" : [ret] "=r"(ret));

    return ret;
}

int 
sem_signal(sem_t id)
{

    if(id == nullptr) return -1;
    uint64 num = SEM_SIGNAL;
    int ret = 0;

    SYSCALL_REG_TWO(num, id);

    ECALL

    asm volatile("mv %[ret], a0" : [ret] "=r"(ret));

    return ret;
}

int 
tcb_create(thread_t* handle, void(*fun)(void*), void* arg)
{

    if(handle == nullptr) return -1;

    uint64 num = TCB_CREATE;
    int ret = 0;
    TCB* res;

    void* stack = nullptr;
    if(fun != nullptr) {
        stack = mem_alloc(DEFAULT_STACK_SIZE);
        if (stack == nullptr) return -1;
    }

    asm volatile("mv a4, %[stack]" : : [stack] "r"(stack));

    SYSCALL_REG_FOUR(num, handle, fun, arg);

    ECALL

    asm volatile("mv %[res], a0" : [res] "=r"(res));

    if(res == nullptr) ret = -1;

    return ret;
}

int 
time_sleep(time_t time)
{

    if(time <= 0) return -1;
    uint64 num = TIME_SLEEP;
    int ret;

    SYSCALL_REG_TWO(num, time);

    ECALL

    asm volatile("mv %[ret], a0" : [ret] "=r"(ret));

    return ret;
}

void 
putc(char c)
{

    uint64 num = PUTC;

    SYSCALL_REG_TWO(num, c);

    ECALL
}

char 
getc()
{

    char c;
    uint64 num = GETC;

    asm volatile("mv a0, %[num]" : : [num] "r"(num));

    ECALL

    asm volatile("mv %[ch], a0" : [ch] "=r"(c));

    return c;
}

void thread_destroy(thread_t handle) {

    uint64 num = THREAD_DESTROY;
    SYSCALL_REG_TWO(num, handle);
    ECALL
}
