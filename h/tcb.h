#ifndef tcb_h
#define tcb_h


#include "MemoryAllocator.h"
#include "system.h"
#include "KernelObject.h"

class Scheduler;
class KernelSem;
class Cache;

class TCB : public KernelObject<TCB> {

public:

    TCB();
    ~TCB();

    struct Context{

        uint64 ra;
        uint64 sp;
    };

    static TCB* usermainThread;
    static TCB* mainThread;

    static TCB* createThread(thread_t* handle, void(*start_routine)(void*), void* arg, void* stack_space, int id, bool start);  // creates the Thread Control Block for given thread handle
    static int startThread(thread_t* handle);

    static void tcbDtor(void* arg);

    bool isStarted() const { return started; }

    void setHolder(KernelSem* sem) { holder = sem; }
    void setClose(int c) { close = c; }

    KernelSem* getHolder() const { return holder; }
    int getClose() const { return close; }

    void setBlocked(bool block) { blocked = block; }
    bool isBlocked() const { return blocked; }

    time_t getTimeSlice() const { return timeSlice; }

    bool isAsleep() const { return asleep; }
    void setAsleep(bool sleep) { asleep = sleep; }

    static void yield();  // save/restore the registers and dispatch
    static int suspend(); // suspends the thread execution

    bool isFinished() const { return finished; }
    void setFinished(bool f) { finished = f; }

    void setPrivilege(int priv) { privilege = priv; }
    int getPrivilege() const { return privilege; }

    void setPid(int id) { pid = id; }

    static TCB* running;
    static Cache* cacheTCB;

    void* operator new(size_t size);
    void* operator new[](size_t size);

    void operator delete(void* addr);
    void operator delete[](void* addr);


private:

    TCB* next, *prev;

    uint64* stack;
    Context context = {0, 0};

    bool finished;
    bool blocked;
    bool started;
    bool asleep;
    int privilege;

    int pid;

    void (*fun)(void*);
    void* funArg;

    friend class Riscv;
    friend class Scheduler;
    friend class KernelSem;

    time_t timeSlice;

    static uint64 timeSliceCounter;

    KernelSem* holder;
    int close;

    static void wrapper(); // wrapper function for the thread body
    static void dispatch();  // interract with scheduler and call the context switch
    static void contextSwitch(Context* oldContext, Context* newContext);  // switch to the context of another thread

    friend class MMU;
};

#endif // tcb_h
