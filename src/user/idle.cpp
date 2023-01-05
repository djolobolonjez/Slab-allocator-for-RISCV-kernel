#include "../../h/idle.h"
#include "../../h/syscall_c.h"

void idle(void*) {
    while(1) { thread_dispatch(); }
};