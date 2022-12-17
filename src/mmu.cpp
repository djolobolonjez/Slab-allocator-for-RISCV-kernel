#include "../h/mmu.h"

uint64* MMU::alignAddress(uint64* addr, uint64 order) {

    uint64 mask = (1 << order) - 1;
    uint64 a = (uint64)addr;
    a += mask;
    a &= (~mask);
    return (uint64*) a;
}