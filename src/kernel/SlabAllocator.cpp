#include "../../h/buddy.h"
#include "../../h/SlabAllocator.h"
#include "../../h/kprint.h"

void Slab::createSlab(size_t size, Cache* handle) {
    Slab* newSlab = (Slab*) Buddy::alloc(size);

    if (newSlab == nullptr) {
        handle->setError(-2 );
        return;
    }

    initSlab(newSlab, handle);

    newSlab->mem = newSlab->freeArray + newSlab->numOfSlots;

    if (handle->ctor)
        for (unsigned i = 0; i < handle->objNum; i++) {
            void* addr = (void*) ((uint8*)newSlab->mem + i * handle->slotSize);
            handle->ctor(addr);
        }

    handle->moveFree(newSlab, Cache::PARTIAL);
}

void* Slab::takeObject(Slab* slab) {

    if (!slab) return nullptr;

    void* objp = (uint8*)slab->mem + slab->free * slab->owner->slotSize;
    slab->free = slab->freeArray[slab->free];

    slab->numOfFreeSlots--;

    return objp;
}

void Slab::putObject(void *objp, Cache* handle) {
    Slab* slabObject = (Slab*) objp;

    uint64 mask = ~0UL << 12;
    Slab* slabHeader = (Slab*)((uint64) slabObject & mask);

    if (slabHeader->owner != handle) {
        handle->setError(-3);
        return;
    }

    updateSlab(slabHeader);

    if (slabHeader->owner->dtor)
        slabHeader->owner->dtor(objp);

    if (slabHeader->owner->ctor)
        slabHeader->owner->ctor(objp);

    unsigned index = ((uint8*)slabObject - (uint8*)slabHeader->mem) / (slabHeader->owner->slotSize);
    slabHeader->freeArray[index] = slabHeader->free;
    slabHeader->free = index;
}

void Slab::destroySlab(Slab *slab) {
    Cache *handle = slab->owner;

    if (handle->dtor)
        for (unsigned i = 0; i < handle->objNum; i++) {
            void *addr = (void *) ((uint8 *) slab->mem + i * handle->slotSize);
            handle->dtor(addr);
        }

    if (handle->type == 0)
        Buddy::free(slab, handle->slabOrder);
    else {
        Buddy::free(slab->mem, handle->slabOrder);
        int index = CachePool::getPowerOfTwo(handle->slotSize);
        Cache* slabCache = CachePool::getSlabCache(index);
        slabCache->cacheFree(slab);
    }
}

void Slab::createBufferSlab(size_t size, Cache *handle) {
    int index = CachePool::getPowerOfTwo(handle->slotSize);
    Cache* slabCache = CachePool::getSlabCache(index);

    if (!slabCache) {
        handle->setError(-1);
        return;
    }

    Slab* newSlab = (Slab*) slabCache->cacheAlloc();

    if (!newSlab) {
        handle->setError(-1);
        return;
    }

    newSlab->mem = Buddy::alloc(size);

    if (newSlab->mem == nullptr) {
        handle->setError(-1);
        return;
    }
    else {    	
    	initSlab(newSlab, handle);
    }

    handle->moveFree(newSlab, Cache::PARTIAL);
}

void Slab::initSlab(Slab *slab, Cache* handle) {

    slab->prev = nullptr;
    slab->next = handle->slabsFree;
    if (handle->slabsFree) handle->slabsFree->prev = slab;
    handle->slabsFree = slab;

    handle->numOfSlabs++;
    slab->numOfSlots = slab->numOfFreeSlots = handle->objNum;

    slab->group = Cache::FREE;

    slab->free = 0;
    slab->freeArray = (unsigned*) (slab + 1);
    slab->owner = handle;

    for (unsigned i = 0; i < handle->objNum - 1; i++)
        slab->freeArray[i] = i + 1;
    slab->freeArray[handle->objNum - 1] = -1;
}

void Slab::putBuffer(void *objp) {
    Slab* slabHeader = nullptr;

    uint64 mask = ~0UL << 12;
    uint8* bufferObject = (uint8*)((uint64)objp & mask);

    for (int buffIndex = 0; buffIndex < BUFFER_NUM; buffIndex++) {
        Cache* bcache = CachePool::getBufferCache(buffIndex);
        if (!bcache) continue;

        Slab* curr = bcache->slabsPartial;
        while (curr) {
            if (curr->mem == bufferObject)
                break;
            curr = curr->next;
        }

        if (curr != nullptr) {
            slabHeader = curr;
            break;
        }

        curr = bcache->slabsFull;
        while (curr) {
            if (curr->mem == bufferObject)
                break;
            curr = curr->next;
        }
        if (curr != nullptr) {
            slabHeader = curr;
            break;
        }
    }

    if (!slabHeader) {
        kprintString("Invalid address for buffer deallocation!");
        return; // Exception: Invalid address!
    }

    updateSlab(slabHeader);

    unsigned index = ((uint8*)objp - (uint8*)slabHeader->mem) / (slabHeader->owner->slotSize);
    slabHeader->freeArray[index] = slabHeader->free;
    slabHeader->free = index;
}

void Slab::updateSlab(Slab *slabHeader) {
    if (slabHeader->numOfFreeSlots == 0) {
        if (slabHeader->numOfSlots == 1)
            slabHeader->owner->moveSlab(slabHeader, Cache::FREE);
        else
            slabHeader->owner->moveSlab(slabHeader, Cache::PARTIAL);
    }
    else {
        if (slabHeader->numOfFreeSlots == (slabHeader->numOfSlots - 1))
            slabHeader->owner->moveSlab(slabHeader, Cache::FREE);
    }

    slabHeader->numOfFreeSlots++;
}
