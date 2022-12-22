#include "../h/riscv.h"
#include "../h/tcb.h"
#include "../h/syscall_cpp.hpp"
#include "../h/kernelcons.h"
#include "../h/scheduler.h"
#include "../h/buddy.h"
#include "../h/slab.h"
#include "../h/CachePool.h"
#include "../h/cache.h"

void userMain();

void main(){

    int blockNum = Buddy::getBlockNum();
    void* space = (void*) (HEAP_START_ADDR);

    kmem_init(space, blockNum);

    Cache* handle1 = CachePool::allocateSlot();
    handle1->flag = 0;

    Cache* handle2 = CachePool::allocateSlot();
    handle2->flag = 0;

    /*char* blk = (char*) Buddy::alloc(0);
    char* blk2 = (char*) Buddy::alloc(0);
    char* blk3 = (char*) Buddy::alloc(1);
    char* blk4 = (char*) Buddy::alloc(1);

    Buddy::free(blk, 0);
    Buddy::free(blk2, 0);
    Buddy::free(blk3, 1);
    Buddy::free(blk4, 1);*/

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
