#include "../h/riscv.h"
#include "../h/tcb.h"
#include "../h/syscall_cpp.hpp"
#include "../h/kernelcons.h"
#include "../h/scheduler.h"
#include "../h/MemoryAllocator.h"
#include "../h/mmu.h"

//#define PAGE_SIZE 4096
#define UNUSED(x) (void)x

char digits[] = "0123456789ABCDEF";

void printInt(int xx, int base, int sgn)
{
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if(sgn && xx < 0){
        neg = 1;
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;
    do{
        buf[i++] = digits[x % base];
    }while((x /= base) != 0);
    if(neg)
        buf[i++] = '-';

    while(--i >= 0)
        putc(buf[i]);

}

void userMain();

void main(){

    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);

    KernelConsole* instance = KernelConsole::getInstance();
    TCB* usermainThread = nullptr, * putcThread = nullptr, *mainThread = nullptr;

    thread_create(&putcThread, KernelConsole::consoleput, nullptr);
    thread_create(&mainThread, nullptr, nullptr);
    thread_create(&Scheduler::idleThread, &Scheduler::idle, nullptr);

    TCB::running = mainThread;

    putcThread->setPrivilege(1);
    mainThread->setPrivilege(1);

    user_main_* wrap = (user_main_*) mem_alloc(sizeof(user_main_));
    wrap->fn = &userMain;

    thread_create(&usermainThread, user_wrapper, wrap);
    usermainThread->setPid(1);

    while(!usermainThread->isFinished()){
        thread_dispatch();
    }

    Riscv::ms_sstatus(SSTATUS_SIE);

    while(instance->inputHead() != instance->inputTail()) { }

    Riscv::mc_sstatus(SSTATUS_SIE);

    putcThread->setFinished(true);
    Scheduler::idleThread->setFinished(true);
    delete putcThread;
    delete Scheduler::idleThread;
    delete usermainThread;
    delete instance;
    mainThread->setFinished(true);
    delete mainThread;

}
