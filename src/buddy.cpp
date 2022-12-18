#include "../h/buddy.h"
#include "../h/system.h"

void* Buddy::KERNEL_START_ADDR = nullptr;
void* Buddy::KERNEL_END_ADDR = nullptr;
int Buddy::MAX_ORDER = 0;
Buddy::FreeArea** Buddy::blocks = nullptr;

size_t Buddy::greaterPowerOfTwo(size_t x, int* order) {
    size_t value = 1;
    while (x >= value){
        value <<= 1;
        (*order)++;
    }
    return value;
}

inline void Buddy::pageAlign() {
    uint64 mask = (1 << MAX_ORDER) - 1;
    uint64 startAddr = ((uint64) KERNEL_START_ADDR + mask) & (~mask);
    uint64 endAddr = ((uint64) KERNEL_END_ADDR + mask) & (~mask);

    KERNEL_START_ADDR = (void*) startAddr;
    KERNEL_END_ADDR = (void*) endAddr;
}

void Buddy::buddyInit() {

    KERNEL_START_ADDR = (void*)HEAP_START_ADDR;
    size_t size = ((char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR) * 125 / 1000;
    size_t numOfBlocks = size / BLOCK_SIZE;
    size_t powerOfTwoBlocks = greaterPowerOfTwo(numOfBlocks, &MAX_ORDER);

    if (powerOfTwoBlocks - numOfBlocks > 0)
        KERNEL_END_ADDR = (char*) KERNEL_START_ADDR + powerOfTwoBlocks * BLOCK_SIZE + (MAX_ORDER + 1) * sizeof (FreeArea*);
    else
        KERNEL_END_ADDR = (char*)KERNEL_START_ADDR + size + (MAX_ORDER + 1) * sizeof (FreeArea*);

    blocks = (FreeArea**)KERNEL_START_ADDR;
    KERNEL_START_ADDR = (char*)KERNEL_START_ADDR + (MAX_ORDER + 1) * sizeof (FreeArea*);
    for (int i = 0; i < MAX_ORDER; i++)
        blocks[i] = nullptr;

    pageAlign();
    HEAP_START_ADDR = (char*)KERNEL_END_ADDR + 1;

    blocks[MAX_ORDER] = (FreeArea*) KERNEL_START_ADDR;
    blocks[MAX_ORDER]->next = nullptr;
}

inline void Buddy::addBlock(char *addr, int order) {
    FreeArea* blk = (FreeArea*) addr;
    if (blocks[order] != nullptr)
        blk->next = blocks[order];
    else
        blk->next = nullptr;

    blocks[order] = blk;
}

inline void* Buddy::getBlock(int order) {
    FreeArea* p = blocks[order];
    if (p) blocks[order] = p->next;
    return p;
}

inline void Buddy::split(char *addr, int upper, int lower) {
    while (--upper >= lower) {
        char* newBlk = addr + (1 << upper) * BLOCK_SIZE;
        addBlock(newBlk, upper);
    }
}

void* Buddy::alloc(int size) {
    if (size < 0 || size > MAX_ORDER)
        return nullptr;

    for (int curSize = size; curSize <= MAX_ORDER; curSize++) {
        char* p = (char*) getBlock(curSize);
        if (!p) continue;
        split(p, curSize, size);
        return p;
    }
    return nullptr;
}