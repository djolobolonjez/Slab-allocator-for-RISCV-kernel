#include "../h/kernelcons.h"
#include "../h/riscv.h"
#include "../h/MemoryAllocator.h"

KernelConsole* KernelConsole::instance = nullptr;

KernelConsole::KernelConsole() {

    sem_open(&emptyBuff1, 0);
    sem_open(&fullBuff1, MAX_BUFF_SIZE);
    sem_open(&emptyBuff2, 0);
    sem_open(&fullBuff2, MAX_BUFF_SIZE);
}

KernelConsole::~KernelConsole() {

    sem_close(emptyBuff2);
    sem_close(emptyBuff1);
    sem_close(fullBuff2);
    sem_close(fullBuff1);

}

KernelConsole* KernelConsole::getInstance() {

    if(instance == nullptr){
        instance = new KernelConsole();
    }
    return instance;
}


void KernelConsole::put(char c) {

    sem_wait(fullBuff1);

    output_buff[tail1] = c;
    tail1 = (tail1 + 1) % MAX_BUFF_SIZE;

}

char KernelConsole::get() {

    sem_wait(emptyBuff2);

    char c = input_buff[head2];
    head2 = (head2 + 1) % MAX_BUFF_SIZE;
    size--;

    return c;
}

void KernelConsole::consoleput(void* arg) {

    while(1){

        KernelConsole* cons = getInstance();

        while(WRITE_READY && cons->head1 != cons->tail1) {

            C_WRITE = cons->output_buff[cons->head1];
            cons->head1 = (cons->head1 + 1) % MAX_BUFF_SIZE;
            sem_signal(cons->fullBuff1);
        }

        plic_complete(CONSOLE_IRQ);
        thread_dispatch();
    }
}


void KernelConsole::operator delete(void* addr) {
    MemoryAllocator::kmem_free(addr);
}

void* KernelConsole::operator new(size_t size) {
    return MemoryAllocator::kmem_alloc(size/MEM_BLOCK_SIZE + (size%MEM_BLOCK_SIZE != 0 ? 1:0));
}
