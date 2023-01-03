#include "../../h/kernelcons.h"
#include "../../h/riscv.h"
#include "../../h/slab.h"
#include "../../h/CachePool.h"

KernelConsole* KernelConsole::instance = nullptr;

KernelConsole::KernelConsole() {

    sem_open(&emptyBuff1, 0);
    sem_open(&fullBuff1, MAX_SIZE);
    sem_open(&emptyBuff2, 0);
    sem_open(&fullBuff2, MAX_SIZE);
    sem_open(&readSem, 0);
    sem_open(&writeSem, 0);

    output_buff = (char*) kmalloc(MAX_SIZE);
    input_buff = (char*) kmalloc(MAX_SIZE);
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
    tail1 = (tail1 + 1) % MAX_SIZE;

    sem_signal(emptyBuff1);
}

char KernelConsole::get() {

    sem_wait(emptyBuff2);

    char c = input_buff[head2];
    head2 = (head2 + 1) % MAX_SIZE;

    sem_signal(fullBuff2);

    return c;
}

void KernelConsole::consoleput(void* arg) {

    while(1) {

        KernelConsole* cons = getInstance();

        sem_wait(cons->writeSem);
        while(WRITE_READY) {

            sem_wait(cons->emptyBuff1); // wait for character
            C_WRITE = cons->output_buff[cons->head1];
            cons->head1 = (cons->head1 + 1) % MAX_SIZE;
            sem_signal(cons->fullBuff1); // space is available
        }
    }
}


void KernelConsole::operator delete(void* addr) {
    kfree(addr);
}

void* KernelConsole::operator new(size_t size) {
    return kmalloc(CachePool::powerOfTwoSize(size));
}

void KernelConsole::flush() const {

    while(inputHead() != inputTail()) { }
}

void KernelConsole::consoleget(void *arg) {

    while (1) {
        KernelConsole* cons = getInstance();
        sem_wait(cons->readSem);
        if (READ_READY) {
            sem_wait(cons->fullBuff2);
            cons->input_buff[cons->tail2] = C_READ;
            cons->tail2 = (cons->tail2 + 1) % MAX_SIZE;
            sem_signal(cons->emptyBuff2);
        }

    }
}
