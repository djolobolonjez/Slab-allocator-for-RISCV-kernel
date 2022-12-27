#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

#include "../lib/hw.h"
#include "../h/cache.h"
#include "../h/system.h"

class Slab {
private:

    Cache* owner;
    Slab* next, *prev;

    void* mem;
    unsigned free;

    unsigned* freeArray;

    size_t numOfSlots;
    size_t numOfFreeSlots;

    Cache::SlabGroup group;

    friend class Cache;
public:
    static void createSlab(size_t size, Cache* handle);
    static void destroySlab(Slab* slab);
    static void* takeObject(Slab* slab);
    static void putObject(void* objp);



};

#endif // SLAB_ALLOCATOR_H