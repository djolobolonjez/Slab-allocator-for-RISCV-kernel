#ifndef _scheduler_h
#define _scheduler_h

class TCB;

class Scheduler{

private:
    Scheduler() = default;

    static TCB* head;
    static TCB* tail;

public:

    friend class TCB;

    static void idle(void*);  // function for idle thread

    static TCB* get();  // take the thread from scheduler

    static void put(TCB* tcb); // put the thread into scheduler

    static TCB* idleThread;

};



#endif
