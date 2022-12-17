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

    static size_t MAX_ORDER;
    static FreeArea** blocks;

    static size_t greaterPowerOfTwo (size_t x, size_t* order);
public:
    static void buddyInit();

};


#endif // _BUDDY_H