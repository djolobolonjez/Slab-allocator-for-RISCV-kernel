#ifndef _kernelcons_h
#define _kernelcons_h

#include "../h/syscall_c.h"

class KernelConsole{

private:
    KernelConsole();

    static KernelConsole* instance;

    char input_buff[MAX_BUFF_SIZE];
    char output_buff[MAX_BUFF_SIZE];

    KernelSem* fullBuff1, *emptyBuff1;
    KernelSem* fullBuff2, *emptyBuff2;

    int head1 = 0, head2 = 0, tail1 = 0, tail2 = 0;

    int size = 0;

public:
    ~KernelConsole();

    int inputHead() const { return head1; }
    int inputTail() const { return tail1; }

    void put(char);  // put the character into output buffer
    char get();  // take the character from the input buffer

    static void consoleput(void* arg);   // kernel function for sending the character to the console

    static KernelConsole* getInstance();  // get the single instance

    friend class Riscv;

    void* operator new(size_t);
    void operator delete(void*);

};

#endif