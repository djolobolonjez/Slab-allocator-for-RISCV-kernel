#include "../../h/scheduler.h"
#include "../../h/tcb.h"
#include "../../h/syscall_c.h"

TCB* Scheduler::head;
TCB* Scheduler::tail;

TCB* Scheduler::idleThread = nullptr;

TCB* Scheduler::get() {

    if(!head) return idleThread;
    TCB* data = head;
    head = head->next;

    data->next = nullptr;
    if(!head) tail = nullptr;
    else head->prev = nullptr;

    return data;

}

void Scheduler::put(TCB* tcb){

    if(tail){
        tail->next = tcb;
        tcb->prev = tail;
        tail = tail->next;
        tcb->next = nullptr;
    }else
    {
        head = tail = tcb;
        head->next = tail->next = nullptr;
        head->prev = tail->prev = nullptr;
    }

}


