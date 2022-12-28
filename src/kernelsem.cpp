#include "../h/scheduler.h"
#include "../h/riscv.h"
#include "../h/system.h"
#include "../h/kernelsem.h"
#include "../h/tcb.h"
#include "../h/slab.h"

Cache* KernelSem::cacheSem = nullptr;

KernelSem::KernelSem() {
    this->head = nullptr;
    this->tail = nullptr;
    this->val = 0;
}

KernelSem::KernelSem(sem_t* handle, unsigned int init_value) : val((int)init_value) {
    head = tail = nullptr;
    *handle = this;
}

KernelSem::~KernelSem() {
    Queue* curr = KernelSem::head, *prev = nullptr;
    while(curr){
        prev = curr;
        curr = curr->next;
        TCB* data = prev->tcb;
        data->setClose(1);
        data->setHolder(nullptr);

        Scheduler::put(data);
        KernelSem::deleteNode(prev);
    }

    head = tail = nullptr;
}

KernelSem* KernelSem::createSem(sem_t* pSem, unsigned int init_value) {

    KernelSem* semaphore = (KernelSem*) kmem_cache_alloc(KernelSem::cacheSem);
    semaphore->head = semaphore->tail = nullptr;
    semaphore->val = (int)init_value;

    *pSem = semaphore;

    return semaphore;
}

int KernelSem::deleteSem(KernelSem* sem) {

    delete sem;
    return 0;
}

KernelSem::Queue* KernelSem::createNode() {
    return (Queue*) MemoryAllocator::kmem_alloc(sizeof(Queue));
}

void KernelSem::deleteNode(Queue* node) {
    MemoryAllocator::kmem_free(node);
}

int KernelSem::wait() {

    if(--val < 0)
        block();

    if(TCB::running->getHolder() == nullptr && TCB::running->getClose() == 1) return -1;

    return 0;
}

int KernelSem::signal() {

    if(val++ < 0)
        deblock();
    return 0;
}

void KernelSem::block() {

    TCB::running->setBlocked(true);
    TCB::running->setHolder(this);
    insert(TCB::running);
    TCB::timeSliceCounter = 0;
    TCB::yield();
}

void KernelSem::deblock() {

    Scheduler::put(remove());
}

void KernelSem::insert(TCB* tcb) {

    Queue* node = createNode();
    node->next = nullptr;
    node->tcb = tcb;
    if(tail){
        tail->next = node;
        tail = tail->next;
    }else
    {
        head = tail = node;
    }
}

TCB* KernelSem::remove() {

    if(!head) return nullptr;
    TCB* data = head->tcb;
    data->setHolder(nullptr);
    data->setBlocked(false);

    Queue* old = head;
    head = head->next;
    if(!head) tail = nullptr;

    deleteNode(old);

    return data;
}

void* KernelSem::operator new(size_t size) { return MemoryAllocator::kmem_alloc(size/MEM_BLOCK_SIZE + (size%MEM_BLOCK_SIZE != 0?1:0));}
void* KernelSem::operator new[](size_t size) { return MemoryAllocator::kmem_alloc(size/MEM_BLOCK_SIZE + (size%MEM_BLOCK_SIZE != 0?1:0));}

void KernelSem::operator delete(void *addr) { /*MemoryAllocator::kmem_free(addr);*/kmem_cache_free(KernelSem::cacheSem, addr); }
void KernelSem::operator delete[](void *addr)  { MemoryAllocator::kmem_free(addr); }
