#ifndef kernelcons_h
#define kernelcons_h

#include "syscall_c.h"

class KernelConsole{

private:
    KernelConsole();

    static KernelConsole* instance;

    char* input_buff;
    char* output_buff;

    KernelSem* txBuff, *txFull;
    KernelSem* rxEmpty, *rxBuff;
    KernelSem* uartReceive, *uartTransmit;

    int outpHead = 0, inpHead = 0, outpTail = 0, inpTail = 0;

public:
    ~KernelConsole();

    int outputHead() const { return outpHead; }
    int outputTail() const { return outpTail; }

    void put(char);  // put the character into output buffer
    char get();  // take the character from the input buffer

    void flush() const;

    static void consoleput(void* arg); // kernel function for sending the character to the console
    static void consoleget(void* arg);

    static KernelConsole* getInstance();// get the single instance

    friend class Riscv;

    void* operator new(size_t);
    void operator delete(void*);

};

#endif // kernelcons_h
