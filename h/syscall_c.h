#ifndef syscall_c
#define syscall_c

#include "system.h"


void* 
mem_alloc(size_t size);

int 
mem_free(void* addr);

int 
thread_create(thread_t* handle, void(*start_routine)(void*), void* arg);

int 
thread_exit();

void 
thread_dispatch();

int 
sem_open(sem_t* handle, unsigned init);

int 
sem_close(sem_t handle);

int 
sem_wait(sem_t id);

int 
sem_signal(sem_t id);

int 
tcb_create(thread_t* handle, void(*fun)(void*), void* arg);

int
thread_start(thread_t* handle);

void
thread_destroy(thread_t handle);

int 
time_sleep(time_t);

char 
getc();

void 
putc(char);


#endif // syscall_c
