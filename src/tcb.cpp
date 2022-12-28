#include "../h/riscv.h"
#include "../h/scheduler.h"
#include "../h/tcb.h"
#include "../h/syscall_c.h"
#include "../h/slab.h"

class Thread;

Cache* TCB::cacheTCB = nullptr;
TCB* TCB::running;
int TCB::call = 0;

uint64 TCB::timeSliceCounter = 0;

TCB::~TCB() {

    if(!isFinished()){
        if(next) next->prev = prev;
        else Scheduler::tail = prev;
        if(prev) prev->next = next;
        else Scheduler::head = next;

    }
    delete[] stack;
    if(pid == 1 && funArg != nullptr) mem_free(funArg);

}

void* TCB::operator new(size_t size) { return MemoryAllocator::kmem_alloc(size/MEM_BLOCK_SIZE + (size%MEM_BLOCK_SIZE != 0 ? 1:0));}
void* TCB::operator new[](size_t size) { return MemoryAllocator::kmem_alloc(size/MEM_BLOCK_SIZE + (size%MEM_BLOCK_SIZE != 0 ? 1:0));}

void TCB::operator delete(void *addr) { MemoryAllocator::kmem_free(addr); }

void TCB::operator delete[](void *addr)  { MemoryAllocator::kmem_free(addr); }


void TCB::yield() {

    Riscv::pushRegisters();

    TCB::dispatch();

    Riscv::popRegisters();

}

void TCB::dispatch() {

    TCB* old = running;

    if(!old->isFinished() && !old->isBlocked() && !old->isAsleep()){
        Scheduler::put(old);
    }

    running = Scheduler::get();

    TCB::contextSwitch(&old->context, &TCB::running->context);
}

TCB::TCB(thread_t *handle, void (*start_routine)(void *), void *arg, void *stack_space, int id) :
                        stack(start_routine != nullptr ? (uint64*)stack_space : nullptr),
                        context({start_routine != nullptr ? (uint64)wrapper : 0,
                                 start_routine != nullptr ? (uint64)&stack[DEFAULT_STACK_SIZE/sizeof(uint64)] : 0})
{
    fun = start_routine;
    funArg = arg;
    timeSlice = DEFAULT_TIME_SLICE;
    holder = nullptr;
    blocked = false;
    asleep = false;
    close = 0;
    next = nullptr;
    prev = nullptr;
    finished = false;
    deleted = false;
    *handle = this;
    privilege = 0;
    pid = id;

    if(start_routine != nullptr && TCB::call == 0) Scheduler::put(this);
}

TCB* TCB::createThread(thread_t *handle, void (*start_routine)(void *), void *arg, void *stack_space, int id) {

    if(TCB::call == 1 && *handle != nullptr) {
        Scheduler::put(*handle);
        return *handle;
    }

    TCB* thread = (TCB*) kmem_cache_alloc(TCB::cacheTCB);
    thread->stack = (start_routine != nullptr ? (uint64*)stack_space : nullptr);
    thread->fun = start_routine;
    thread->funArg = arg;
    thread->pid = id;
    thread->context.ra = (start_routine != nullptr ? (uint64)wrapper : 0);
    thread->context.sp = (start_routine != nullptr ? (uint64)&thread->stack[DEFAULT_STACK_SIZE/sizeof(uint64)] : 0);
    *handle = thread;

    if(start_routine != nullptr && TCB::call == 0) Scheduler::put(thread);

    return thread;
}

void TCB::wrapper() {

    if(TCB::running->getPrivilege() == 0)
        Riscv::sppUser();
    else
        Riscv::sppKernel();
    TCB::running->fun(TCB::running->funArg);
    thread_exit();
}

int TCB::suspend() {
    if(TCB::running){
        TCB::running->setFinished(true);
        TCB::dispatch();
        return 0;
    }
    return -1;
}

TCB::TCB() {
    this->next = nullptr;
    this->prev = nullptr;
    this->asleep = false;
    this->blocked = false;
    this->close = 0;
    this->deleted = false;
    this->finished = false;
    this->holder = nullptr;
    this->privilege = 0;
    this->pid = 0;
    this->timeSlice = DEFAULT_TIME_SLICE;
    this->stack = nullptr;
    this->fun = nullptr;
    this->funArg = nullptr;
}
