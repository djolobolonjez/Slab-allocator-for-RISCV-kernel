#ifndef sleeping_h
#define sleeping_h

#include "../lib/hw.h"

class TCB;
class Cache;

class Sleeping {
private:
    Sleeping() = default;

    struct SleepQueue {

        TCB* tcb;
        SleepQueue* prev;
        SleepQueue* next;
        time_t time;
    };

    static SleepQueue* head;

    static Cache* cacheSleep;

    static SleepQueue* getNode();  // creates a node for sleeping queue
    static void deleteNode(SleepQueue*);  // delete the node when the thread wakes up

public:

    static void remove(); // remove the thread from sleeping queue
    static void add(time_t);  // put the thread into sleeping queue
};


#endif // sleeping_h