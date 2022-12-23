#include "../h/cache.h"
#include "../h/buddy.h"

void Slab::createSlab(size_t size, Cache* handle) {
    Slab* newSlab = (Slab*) Buddy::alloc(size);

    newSlab->prev = nullptr;
    newSlab->next = handle->slabsFree;
    handle->slabsFree = newSlab;

    newSlab->numOfSlots = handle->objNum;
    newSlab->free = 0;
    newSlab->freeArray = (unsigned*) (newSlab + 1);
    newSlab->owner = handle;
    newSlab->mem = newSlab->freeArray + newSlab->numOfSlots;

    for (unsigned i = 0; i < newSlab->numOfSlots - 1; i++)
        newSlab->freeArray[i] = i + 1;
    newSlab->freeArray[newSlab->numOfSlots - 1] = -1;

    handle->setEmptyToPartial(newSlab);
}

