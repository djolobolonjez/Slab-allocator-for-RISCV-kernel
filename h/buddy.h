#ifndef _BUDDY_H
#define _BUDDY_H

#include "../lib/hw.h"

class Buddy {
private:
    Buddy() = default;

    struct FreeArea {
        FreeArea* next;
    };


    static void* KERNEL_START_ADDR;
    static void* KERNEL_END_ADDR;

    static int MAX_ORDER;
    static FreeArea** blocks;

    static size_t greaterPowerOfTwo (size_t x, int* order);

    static inline void split(char* addr, int upper, int lower);
    static inline void addBlock(char* addr, int order);
    static inline void* getBlock(int order);
public:
    static void buddyInit();
    static void* alloc(int size);

};


#endif // _BUDDY_H