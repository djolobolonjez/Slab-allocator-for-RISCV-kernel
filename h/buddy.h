#ifndef BUDDY_H
#define BUDDY_H

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
    static uint8* bitmap;

    static size_t greaterPowerOfTwo (size_t x, int* order);

    static void splitBlock(uint8* addr, int upper, int lower);
    static void addBlock(uint8* addr, int order);
    static void* getBlock(int order);
    static void pageAlign();
    static void flipBit(int index);
    static void flipParent(void* addr, int order);
    static int getIndex(void* addr, int order);
    static bool isBuddyFree(int index);
    static FreeArea* returnBlock(int size, FreeArea* addr);
    static FreeArea* coalesceBuddy(int order, int index, FreeArea* addr);

    friend class MMU;
public:
    static int getBlockNum();
    static int buddyInit(void* space, int blockNum);
    static void* alloc(int order);
    static int free (void* addr, int order);

};


#endif // BUDDY_H
