#include "../../h/riscv.h"
#include "../../h/tcb.h"
#include "../../h/syscall_cpp.hpp"
#include "../../h/kernelcons.h"
#include "../../h/scheduler.h"
#include "../../h/buddy.h"
#include "../../h/slab.h"
#include "../../h/cache.h"
#include "../../h/SlabAllocator.h"
#include "../../h/kernelsem.h"
#include "../../h/mmu.h"
#include "../../h/sleeping.h"
#include "../../h/idle.h"

void userMain();

int main() {
    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);

    int blockNum = Buddy::getBlockNum();
    void* space = (void*) (HEAP_START_ADDR);

    kmem_init(space, blockNum);

    MMU::MMUInit();

    uint64 rootPPN = ((uint64) MMU::rootTablePointer) >> 12;
    uint64 satp = (8UL << 60) | rootPPN;

    Riscv::ms_sstatus(  SSTATUS_SUM);
    asm ("csrw satp, %0" :: "r"(satp));

    TCB::cacheTCB = kmem_cache_create("TCB Cache", sizeof(TCB), TCB::ctor, nullptr);
    KernelSem::cacheSem = kmem_cache_create("Semaphore Cache", sizeof(KernelSem), KernelSem::ctor, nullptr);

    KernelConsole* console = KernelConsole::getInstance();
    TCB *putcThread = nullptr, *getcThread = nullptr;

    thread_create(&putcThread, KernelConsole::consoleput, nullptr);
    thread_create(&getcThread, KernelConsole::consoleget, nullptr);
    thread_create(&TCB::mainThread, nullptr, nullptr);
    thread_create(&Scheduler::idleThread, idle, nullptr);
    Scheduler::idleThread->setName("idle");


    TCB::running = TCB::mainThread;

    putcThread->setPrivilege(1);
    putcThread->setName("putc");
    getcThread->setPrivilege(1);
    getcThread->setName("getc");
    TCB::mainThread->setPrivilege(1);
    TCB::mainThread->setName("main");
    user_main_* wrap = (user_main_*) mem_alloc(sizeof(user_main_));
    wrap->fn = &userMain;

    thread_create(&TCB::usermainThread, user_wrapper, wrap);
    TCB::usermainThread->setPid(1);
    TCB::usermainThread->setName("usermain");

    //TCB::usermainThread->setPrivilege(1); //TODO - Postaviti ovaj flag kada se testira iz sistemskog rezima

    while(!TCB::usermainThread->isFinished()){
        thread_dispatch();
    }

    Riscv::ms_sstatus(SSTATUS_SIE);

    console->flush();

    Riscv::mc_sstatus(SSTATUS_SIE);

    putcThread->setFinished(true);
    getcThread->setFinished(true);
    //Scheduler::idleThread->setFinished(true);

    asm volatile ("csrw satp, zero");

    delete console;

    TCB::mainThread->setFinished(true);

    KernelSem::semDestroy();
    Sleeping::sleepDestroy();

    kmem_cache_destroy(TCB::cacheTCB);
    kmem_cache_destroy(KernelSem::cacheSem);

    CachePool::SlabFinalize();
    CachePool::CachePoolFinalize();

    MMU::MMUFinalize();

    return 0;
}
