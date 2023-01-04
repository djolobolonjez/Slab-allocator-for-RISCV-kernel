#include "../../h/kernelcons.h"
#include "../../h/riscv.h"
#include "../../h/slab.h"
#include "../../h/CachePool.h"

KernelConsole* KernelConsole::instance = nullptr;

KernelConsole::KernelConsole() {

    sem_open(&txFull, 0);
    sem_open(&txBuff, MAX_SIZE);
    sem_open(&rxBuff, 0);
    sem_open(&rxEmpty, MAX_SIZE);
    sem_open(&uartReceive, 0);
    sem_open(&uartTransmit, 0);

    output_buff = (char*) kmalloc(MAX_SIZE);
    input_buff = (char*) kmalloc(MAX_SIZE);
}

KernelConsole::~KernelConsole() {

    sem_close(rxBuff);
    sem_close(txFull);
    sem_close(rxEmpty);
    sem_close(txBuff);
	sem_close(uartTransmit);
	sem_close(uartReceive);

}

KernelConsole* KernelConsole::getInstance() {

    if(instance == nullptr){
        instance = new KernelConsole();
    }
    return instance;
}


void KernelConsole::put(char c) {

    sem_wait(txBuff);

    output_buff[outpTail] = c;
    outpTail = (outpTail + 1) % MAX_SIZE;

    sem_signal(txFull);
}

char KernelConsole::get() {

    sem_wait(rxBuff);

    char c = input_buff[inpHead];
    inpHead = (inpHead + 1) % MAX_SIZE;

    sem_signal(rxEmpty);

    return c;
}

void KernelConsole::consoleput(void* arg) {

    while(1) {

        KernelConsole* cons = getInstance();

        sem_wait(uartTransmit);
        while(WRITE_READY) {

            sem_wait(cons->txFull); // wait for character
            C_WRITE = cons->output_buff[cons->outpHead];
            cons->outpHead = (cons->outpHead + 1) % MAX_SIZE;
            sem_signal(cons->txBuff); // space is available
        }
    }
}

void KernelConsole::consoleget(void *arg) {

    while (1) {
        KernelConsole* cons = getInstance();
        sem_wait(uartReceive);
        if (READ_READY) {
            sem_wait(cons->rxEmpty);
            cons->input_buff[cons->inpTail] = C_READ;
            cons->inpTail = (cons->inpTail + 1) % MAX_SIZE;
            sem_signal(cons->rxBuff);
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

    while(outputHead() != outputTail()) { }
}