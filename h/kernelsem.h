#ifndef KERNEL_SEM_H
#define KERNEL_SEM_H

#include "../lib/hw.h"
#include "KernelObject.h"
#include "system.h"

class TCB;
class Cache;

class KernelSem : public KernelObject<KernelSem> {

public:
    KernelSem();
    ~KernelSem();

    int wait();  // wait on the semaphore
    int signal(); // send signal so one thread can pass the semaphore

    unsigned value() const { return val; }

    static KernelSem* createSem(sem_t* pSem, unsigned init_value); // creates a semaphore

    static int deleteSem(KernelSem*);  // deletes a semaphore
    static void semDestroy();

    static Cache* cacheSem;

    void* operator new(size_t size);
    void* operator new[](size_t size);

    void operator delete(void* addr);
    void operator delete[](void* addr);

private:

    struct BlockedQueue {
        TCB* tcb;
        BlockedQueue* next;
    };

    static Cache* cacheBlocked;

    static BlockedQueue* createNode();  // creates a node for the blocked queue implemented as a linked list
    static void deleteNode(BlockedQueue*);  // delete the node when the thread is unblocked

    void block();  // place the thread in blocked queue
    void deblock();  // remove thread from blocked queue and place it back to scheduler

    void insert(TCB*); // helper function
    TCB* remove();  // helper

    int val;

    BlockedQueue* head, *tail;

};


#endif // KERNEL_SEM_H
