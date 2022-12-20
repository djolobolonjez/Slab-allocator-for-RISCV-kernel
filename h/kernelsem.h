#ifndef KERNEL_SEM_H
#define KERNEL_SEM_H

#include "../lib/hw.h"

class TCB;

class KernelSem{

public:

    ~KernelSem();

    int wait();  // wait on the semaphore
    int signal(); // send signal so one thread can pass the semaphore

    unsigned value() const { return val; }

    static KernelSem* createSem(sem_t* pSem, unsigned init_value); // creates a semaphore

    static int deleteSem(KernelSem*);  // deletes a semaphore

    void* operator new(size_t size);
    void* operator new[](size_t size);

    void operator delete(void* addr);
    void operator delete[](void* addr);

private:

    KernelSem(sem_t* pSem, unsigned init_value);

    struct Queue{
        TCB* tcb;
        Queue* next;
    };

    static Queue* createNode();  // creates a node for the blocked queue implemented as a linked list
    static void deleteNode(Queue*);  // delete the node when the thread is unblocked

    void block();  // place the thread in blocked queue
    void deblock();  // remove thread from blocked queue and place it back to scheduler

    void insert(TCB*); // helper function
    TCB* remove();  // helper

    int val;

    Queue* head, *tail;

};


#endif // KERNEL_SEM_H
