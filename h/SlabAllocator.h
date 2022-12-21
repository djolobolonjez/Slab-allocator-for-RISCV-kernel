#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

class Cache;

class Slab {
private:
    Cache* owner;
    Slab* next, *prev;

    void* slots;

    int numOfSlots;
    int numOfFreeSlots;

    char* isFree;
};

#endif // SLAB_ALLOCATOR_H