#include "../../h/syscall_cpp.hpp"
#include "../../h/tcb.h"

void* operator new(size_t size){

    return mem_alloc(size);
}
void operator delete(void* addr) {

    mem_free(addr);
}

void* operator new[](size_t size){

    return mem_alloc(size);
}

void operator delete[](void* addr){

    mem_free(addr);
}

void user_wrapper(void* arg){
    user_main_* ptr = (user_main_*)arg;
    ptr->fn();
}

void run_wrapper(void* ptr){
    thread_* thr = (thread_*)ptr;
    ((thr->t)->*(thr->fn))();
}

void periodic_wrapper(void* ptr){
    periodic_thread* pthr = (periodic_thread*)ptr;
    while(true){
        ((pthr->pt)->*(pthr->periodic_fn))();
        time_sleep(pthr->period);
    }
}

Thread::Thread(void (*body)(void*), void *arg) : myHandle(nullptr) {
    tcb_create(&myHandle, body, arg);
}

Thread::Thread() : myHandle(nullptr) {
    thread_* thr = (thread_*)mem_alloc(sizeof(thread_));
    if (thr != nullptr) {
        thr->fn = &Thread::run;
        thr->t = this;
        tcb_create(&myHandle, run_wrapper, thr);
    }
}


int Thread::start() {
    return thread_start(&myHandle);
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t time) {
    return time_sleep(time);
}

Thread::~Thread() { thread_destroy(myHandle); }

Semaphore::Semaphore(unsigned int init) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() { sem_destroy(myHandle); }

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}

PeriodicThread::PeriodicThread(time_t period) :
    Thread(periodic_wrapper, new periodic_thread{&PeriodicThread::periodicActivation, this, period}) { }
