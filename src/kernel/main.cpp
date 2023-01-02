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
    TCB* usermainThread = nullptr, *putcThread = nullptr, *mainThread = nullptr;

    TCB::createThread(&putcThread, KernelConsole::consoleput, nullptr, kmalloc(DEFAULT_STACK_SIZE), 0);

    thread_create(&mainThread, nullptr, nullptr);
    thread_create(&Scheduler::idleThread, Scheduler::idle, nullptr);

    TCB::running = mainThread;

    putcThread->setPrivilege(1);
    mainThread->setPrivilege(1);

    kmem_cache_t* handle = kmem_cache_create("TCB Cache", sizeof(TCB), nullptr, nullptr);

    TCB* thread_one = (TCB*) kmem_cache_alloc(handle);
    for (int i = 0; i < 100; i++)
        kmem_cache_alloc(handle);
    TCB* thread_two = (TCB*) kmem_cache_alloc(handle);

    kmem_cache_info(handle);

    kmem_cache_free(handle, thread_one);
    kmem_cache_free(handle, thread_two);

    kmem_cache_destroy(handle);

    char* buff = (char*) kmalloc(MIN_BUFF_SIZE);
    kfree(buff);

    // TODO - Odraditi dealokaciju svih rucki kesa i vratiti sve Badiju

    user_main_* wrap = (user_main_*) mem_alloc(sizeof(user_main_));
    wrap->fn = &userMain;

    thread_create(&usermainThread, user_wrapper, wrap);
    usermainThread->setPid(1);

    while(!usermainThread->isFinished()){
        thread_dispatch();
    }

    Riscv::ms_sstatus(SSTATUS_SIE);

    console->flush();

    Riscv::mc_sstatus(SSTATUS_SIE);

    putcThread->setFinished(true);
    Scheduler::idleThread->setFinished(true);

    asm volatile ("csrw satp, zero");

    // TODO - deallocate all page tables

    /*delete putcThread;
    delete Scheduler::idleThread;
    delete usermainThread;*/
    delete console;
    mainThread->setFinished(true);
    //delete mainThread; // TODO - srediti dealokaciju!!!

    kmem_cache_destroy(Sleeping::cacheSleep);
    kmem_cache_destroy(TCB::cacheTCB);
    kmem_cache_destroy(KernelSem::cacheSem);

    return 0;
}
