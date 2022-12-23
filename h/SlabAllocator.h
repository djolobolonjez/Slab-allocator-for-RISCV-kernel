#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

#include "../lib/hw.h"

class Cache;

class Slab {
private:

    Cache* owner;
    Slab* next, *prev;

    void* mem;
    unsigned free;

    unsigned* freeArray;

    size_t numOfSlots;
    size_t numOfFreeSlots;

    friend class Cache;
public:
    static void createSlab(size_t size, Cache* handle);
};

#endif // SLAB_ALLOCATOR_H