#ifndef _defs_h
#define _defs_h

const int EOF = -1;

class TCB;
class KernelSem;
class Thread;
class PeriodicThread;

typedef KernelSem* sem_t;

typedef TCB* thread_t;

typedef void (Thread::*run_ptr)();

typedef void(*user)();

typedef void (PeriodicThread::*periodic_ptr)();

struct thread_ {
    run_ptr fn;
    Thread* t;
};

struct periodic_thread {

    periodic_ptr periodic_fn;
    PeriodicThread* pt;
    time_t period;
};

struct user_main_ {
    user fn;
};



#endif