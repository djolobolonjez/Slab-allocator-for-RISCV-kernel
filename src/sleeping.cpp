#include "../h/sleeping.h"
#include "../h/scheduler.h"
#include "../h/tcb.h"

Sleeping::SleepQueue* Sleeping::head = nullptr;

Sleeping::SleepQueue* Sleeping::getNode() {

    SleepQueue* node = (SleepQueue*) MemoryAllocator::kmem_alloc(sizeof(SleepQueue));
    node->next = node->prev = nullptr;
    return node;
}

void Sleeping::deleteNode(SleepQueue* node) {
    MemoryAllocator::kmem_free(node);
}

void Sleeping::remove(){

    if(head){
        head->time--;
        SleepQueue* curr = head;
        while(curr){
            if(curr->time == 0){

                curr->tcb->setAsleep(false);
                Scheduler::put(curr->tcb);
                SleepQueue* old = curr;
                curr = curr->next;
                head = curr;
                deleteNode(old);
            }
            else break;
        }
    }
}

void Sleeping::add(time_t time) {

    SleepQueue* node = getNode();
    node->time = time;
    node->tcb = TCB::running;

    if(!head) head = node;
    else
    {
        if(node->time < head->time){
            node->next = head;
            node->next->time -= node->time;
            head->prev = node;
            head = node;
        }
        else
        {
            SleepQueue* curr = head;
            node->time -= curr->time;
            while(curr && curr->next){

                if(node->time < curr->next->time){
                    node->next = curr->next;
                    curr->next->prev = node;
                    curr->next = node;
                    node->prev = curr;
                    node->next->time -= node->time;
                    break;
                }
                else
                {
                    node->time -= curr->next->time;
                    curr = curr->next;
                }
            }
            if(!curr->next){
                curr->next = node;
                node->prev = curr;
            }

        }

    }
}

