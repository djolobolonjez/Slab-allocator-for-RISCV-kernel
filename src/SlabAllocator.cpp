#include "../h/buddy.h"
#include "../h/SlabAllocator.h"


void Slab::createSlab(size_t size, Cache* handle) {
    Slab* newSlab = (Slab*) Buddy::alloc(size);

    newSlab->prev = nullptr;
    newSlab->next = handle->slabsFree;
    if (handle->slabsFree) handle->slabsFree->prev = newSlab;
    handle->slabsFree = newSlab;

    handle->numOfSlabs++;
    newSlab->numOfSlots = newSlab->numOfFreeSlots = handle->objNum;

    newSlab->group = Cache::FREE;

    newSlab->free = 0;
    newSlab->freeArray = (unsigned*) (newSlab + 1);
    newSlab->owner = handle;
    newSlab->mem = newSlab->freeArray + newSlab->numOfSlots;

    for (unsigned i = 0; i < handle->objNum - 1; i++)
        newSlab->freeArray[i] = i + 1;
    newSlab->freeArray[handle->objNum - 1] = -1;

    if (handle->ctor)
        for (unsigned i = 0; i < handle->objNum; i++) {
            void* addr = (void*) ((uint8*)newSlab->mem + i * handle->slotSize);
            handle->ctor(addr);
        }

    handle->moveFree(newSlab, Cache::PARTIAL);
}

void* Slab::takeObject(Slab* slab) {

    void* objp = (uint8*)slab->mem + slab->free * slab->owner->slotSize;
    slab->free = slab->freeArray[slab->free];

    slab->numOfFreeSlots--;

    return objp;
}

void Slab::putObject(void *objp) {
    Slab* slabObject = (Slab*) objp;

    uint64 mask = ~0UL << 12;
    Slab* slabHeader = (Slab*)((uint64) slabObject & mask);

    if (slabHeader->numOfFreeSlots == 0) {
        if (slabHeader->numOfSlots == 1)
            slabHeader->owner->moveSlab(slabHeader, Cache::FREE);
        else
            slabHeader->owner->moveSlab(slabHeader, Cache::PARTIAL);
    }
    else {
        if (slabHeader->numOfFreeSlots == slabHeader->numOfSlots - 1)
            slabHeader->owner->moveSlab(slabHeader, Cache::FREE);
    }

    slabHeader->numOfFreeSlots++;
    unsigned index = ((uint8*)slabObject - (uint8*)slabHeader->mem) / (slabHeader->owner->slotSize);
    slabHeader->freeArray[index] = slabHeader->free;
    slabHeader->free = index;
}