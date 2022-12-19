#include "../h/buddy.h"
#include "../h/system.h"

void* Buddy::KERNEL_START_ADDR = nullptr;
void* Buddy::KERNEL_END_ADDR = nullptr;
int Buddy::MAX_ORDER = 0;
Buddy::FreeArea** Buddy::blocks = nullptr;
uint8* Buddy::bitmap = nullptr;

size_t Buddy::greaterPowerOfTwo(size_t x, int* order) {
    size_t value = 1;
    while (x >= value){
        value <<= 1;
        (*order)++;
    }
    return value;
}

/*inline*/ void Buddy::pageAlign() {
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
        KERNEL_END_ADDR = (char*) KERNEL_START_ADDR + powerOfTwoBlocks * BLOCK_SIZE + (MAX_ORDER + 1) * sizeof (FreeArea*) + (1 << MAX_ORDER) / sizeof(uint8*);
    else
        KERNEL_END_ADDR = (char*)KERNEL_START_ADDR + size + (MAX_ORDER + 1) * sizeof (FreeArea*) + (1 << MAX_ORDER) / sizeof(uint8*);

    blocks = (FreeArea**)KERNEL_START_ADDR;
    bitmap = (uint8*) ((char*) KERNEL_START_ADDR + (MAX_ORDER + 1) * sizeof(FreeArea*));

    KERNEL_START_ADDR = (char*)KERNEL_START_ADDR + (MAX_ORDER + 1) * sizeof (FreeArea*) + (1 << MAX_ORDER) / sizeof(uint8*);
    for (int i = 0; i < MAX_ORDER; i++)
        blocks[i] = nullptr;

    pageAlign();
    HEAP_START_ADDR = (char*)KERNEL_END_ADDR + 1;

    blocks[MAX_ORDER] = (FreeArea*) KERNEL_START_ADDR;
    blocks[MAX_ORDER]->next = nullptr;
}

/*inline*/ int Buddy::getIndex(void* addr, int order) {

    int entryIndex = (1 << MAX_ORDER) - (1 << (MAX_ORDER - order));
    size_t entryAddr = (char*)addr - (char*)KERNEL_START_ADDR;
    int offset = entryAddr >> (MAX_ORDER + order + 1);

    return entryIndex + offset;
}

/*inline*/ void Buddy::flipBit(int index) {
    bitmap[index / 8] ^= (1 << (index % 8));
}

/*inline*/ bool Buddy::isBuddyFree(int index) {
    uint8 mask = (1 << (index % 8));
    return (bitmap[index / 8] & mask) == 0;
}

/*inline*/ void Buddy::addBlock(char *addr, int order) {
    FreeArea* blk = (FreeArea*) addr;
    if (blocks[order] != nullptr)
        blk->next = blocks[order];
    else
        blk->next = nullptr;

    blocks[order] = blk;
}

/*inline*/ void* Buddy::getBlock(int order) {
    FreeArea* p = blocks[order];
    if (p) blocks[order] = p->next;
    return p;
}

/*inline*/ void Buddy::flipParent(void* addr, int order) {
    if (order < MAX_ORDER) {
        int index = getIndex(addr, order);
        flipBit(index);
    }
}

/*inline*/ Buddy::FreeArea* Buddy::coalesceBuddy(int order, int index, FreeArea *addr) {

    FreeArea* prev = nullptr, * curr = blocks[order];
    while (curr) {
        int ind = getIndex(curr, order);
        if (ind == index) {
            if (!prev) blocks[order] = curr->next;
            else prev->next = curr->next;

            break;
        }
        prev = curr;
        curr = curr->next;
    }

    FreeArea* ret = (addr > curr ? curr : addr);
    return ret;
}

/*inline*/ Buddy::FreeArea* Buddy::returnBlock(int order, FreeArea *addr) {

    addr->next = nullptr;
    int index = getIndex(addr, order);
    flipBit(index);

    if (!blocks[order]) {
        blocks[order] = addr;
        return nullptr;
    }

    if (isBuddyFree(index))
        return coalesceBuddy(order, index, addr);

    addr->next = blocks[order];
    blocks[order] = addr;

    return nullptr;
}

/*inline*/ void Buddy::splitBlock(char *addr, int upper, int lower) {
    flipParent(addr, upper + 1);

    while (--upper >= lower) {
        char* newBlk = addr + (1 << upper) * BLOCK_SIZE;
        addBlock(newBlk, upper);
        flipParent(addr, upper + 1);
    }
}

void* Buddy::alloc(int order) {
    if (order < 0 || order > MAX_ORDER)
        return nullptr; // bad alloc - invalid size!

    for (int curSize = order; curSize <= MAX_ORDER; curSize++) {
        char* p = (char*) getBlock(curSize);
        if (!p) continue;
        splitBlock(p, curSize, order);
        if (order != MAX_ORDER) flipBit(getIndex(p, order));
        return p;
    }
    return nullptr;
}

int Buddy::free(void *addr, int order) {
    if (addr == nullptr) {
        return -1; // Exception
    }

    FreeArea* curAddr = (FreeArea*) addr, * freeAddr;

    for (int curSize = order; curSize <= MAX_ORDER; curSize++) {
        freeAddr = returnBlock(curSize, curAddr);
        if (freeAddr) curAddr = freeAddr;
        else break;
    }

    return 0;
}