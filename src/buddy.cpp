#include "../h/buddy.h"
#include "../h/system.h"

void* Buddy::KERNEL_START_ADDR = nullptr;
void* Buddy::KERNEL_END_ADDR = nullptr;
size_t Buddy::MAX_ORDER = 0;
Buddy::FreeArea** Buddy::blocks = nullptr;

size_t Buddy::greaterPowerOfTwo(size_t x, size_t* order) {
    size_t value = 1;
    while (x >= value){
        value <<= 1;
        (*order)++;
    }
    return value;
}

void Buddy::buddyInit() {
    KERNEL_START_ADDR = (void*)HEAP_START_ADDR;
    size_t size = ((char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR) * 125 / 1000;
    size_t numOfBlocks = size / BLOCK_SIZE;
    size_t powerOfTwoBlocks = greaterPowerOfTwo(numOfBlocks, &MAX_ORDER);

    if (powerOfTwoBlocks - numOfBlocks > 0)
        KERNEL_END_ADDR = (char*) KERNEL_START_ADDR + powerOfTwoBlocks * BLOCK_SIZE;
    else
        KERNEL_END_ADDR = (char*)KERNEL_START_ADDR + size;

    HEAP_START_ADDR = (char*)KERNEL_END_ADDR + 1;

    blocks = (FreeArea**)KERNEL_START_ADDR;
    KERNEL_START_ADDR = (char*)KERNEL_START_ADDR + MAX_ORDER * sizeof (FreeArea*);
    blocks[MAX_ORDER - 1] = (FreeArea*) KERNEL_START_ADDR;
}