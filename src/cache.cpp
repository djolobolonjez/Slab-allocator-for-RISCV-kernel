#include "../h/cache.h"
#include "../h/system.h"
#include "../h/SlabAllocator.h"
#include "../h/printing.hpp"

Cache::Cache(const char *name, size_t size, void (*ctor)(void *), void (*dtor)(void *)) {
    if (!CachePool::cacheTail) {
        CachePool::cacheHead = CachePool::cacheTail = this;
        CachePool::cacheHead->next = CachePool::cacheTail->next = nullptr;
        CachePool::cacheHead->prev = CachePool::cacheTail->prev = nullptr;
    }
    else {
        this->prev = CachePool::cacheTail;
        this->next = nullptr;
        CachePool::cacheTail = CachePool::cacheTail->next = this;
    }

    this->name = name;
    this->slotSize = size;
    this->ctor = ctor;
    this->dtor = dtor;
    this->shrink = 1;

    this->slabOrder = estimateOrder(this->slotSize);
    this->slabSize = (1 << this->slabOrder) * BLOCK_SIZE;
    this->objNum = getNumberOfObjects(this->slabSize, this->slotSize);

    this->numOfSlabs = 0;

    this->slabsFull = nullptr;
    this->slabsPartial = nullptr;
    this->slabsFree = nullptr;
}

Cache::~Cache() {
    destroyCache(this);
}

void* Cache::operator new(size_t size) {
    return CachePool::allocateSlot();
}

void Cache::operator delete(void *addr)  {
    CachePool::deallocateSlot((Cache*)addr);
}

size_t Cache::estimateOrder(size_t slotSize) {
    size_t slabOrder = (slotSize > BLOCK_SIZE ? slotSize / BLOCK_SIZE : 0);

    return slabOrder;
}

size_t Cache::getNumberOfObjects(size_t slabSize, size_t slotSize) {
    int numObj = 0;
    while (numObj * slotSize + numObj * sizeof(unsigned) + sizeof (Slab) <= slabSize)
        numObj++;

    return numObj - 1;
}

void Cache::moveFree(Slab* slab, SlabGroup grp) {

    this->slabsFree = this->slabsFree->next;
    if (this->slabsFree) this->slabsFree->prev = nullptr;
    slab->prev = nullptr;

    Slab* header = (grp == Cache::PARTIAL ? this->slabsPartial : this->slabsFull);

    slab->group = grp;
    slab->next = header;
    if (header) header->prev = slab;
    if (grp == Cache::PARTIAL)
        this->slabsPartial = slab;
    else
        this->slabsFull = slab;
}

void Cache::moveSlab(Slab *slab, SlabGroup grp) {

    Slab* curr = (slab->group == Cache::PARTIAL ? this->slabsPartial : this->slabsFull);
    Slab* header = nullptr;
    if (slab->group == Cache::PARTIAL)
        header = (grp == Cache::FREE ? this->slabsFree : this->slabsFull);
    else
        header = (grp == Cache::PARTIAL ? this->slabsPartial : this->slabsFree);

    if (slab->prev == nullptr) {
        curr = curr->next;
        if (curr) curr->prev = nullptr;
        if (slab->group == Cache::PARTIAL)
            this->slabsPartial = curr;
        else
            this->slabsFull = curr;
    }
    else {
        slab->prev->next = slab->next;
        if (slab->next) slab->next->prev = slab->prev;
    }

    slab->group = grp;
    slab->prev = nullptr;
    slab->next = header;
    if (header) header->prev = slab;

    if (grp == Cache::FREE)
        this->slabsFree = slab;
    else if (grp == Cache::PARTIAL)
        this->slabsPartial = slab;
    else
        this->slabsFull = slab;
}

void* Cache::cacheAlloc() {

    size_t oldNumOfSlabs = this->numOfSlabs;

    if (this->slabsPartial == nullptr && this->slabsFree == nullptr)
        Slab::createSlab(this->slabOrder, this);
    else {
        if (this->slabsPartial == nullptr)
            moveFree(this->slabsFree, Cache::PARTIAL);
    }

    if (this->numOfSlabs > oldNumOfSlabs && this->numOfSlabs > 1)
        setShrink(0);
    void* objp = Slab::takeObject(this->slabsPartial);
    if (this->slabsPartial->numOfFreeSlots == 0)
        moveSlab(this->slabsPartial, Cache::FULL);

    return objp;
}

void Cache::cacheFree(void* objp) {
    Slab::putObject(objp);
    if (this->ctor)
        this->ctor(objp);
}

int Cache::cacheShrink() {
    int count = 0;
    if (shrink == 0)
        setShrink(1);
    else {
        Slab* curr = this->slabsFree;
        while (curr) {
            this->numOfSlabs--;
            Slab* old = curr;
            int numOfBlocks = old->owner->slabSize / BLOCK_SIZE;
            curr = curr->next;
            Slab::destroySlab(old);
            count += numOfBlocks;
        }
    }

    return count;
}

void Cache::destroyCache(Cache *cachep) {

    Slab* curr = cachep->slabsFree;
    deallocSlabGroup(curr);

    curr = cachep->slabsPartial;
    deallocSlabGroup(curr);

    curr = cachep->slabsFull;
    deallocSlabGroup(curr);
}

void Cache::deallocSlabGroup(Slab *slab) {
    Slab* curr = slab;
    while (curr) {
        Slab* old = curr;
        curr = curr->next;
        Slab::destroySlab(old);
    }
}

void Cache::printInfo() {
    printString("Cache name: ");
    printString(this->name);
    printString("\n");

    printString("Object size: ");
    printInt(this->slotSize);
    printString("B\n");

    printString("Cache size: ");
    int cacheSize = (int)(this->numOfSlabs * this->slabSize) / BLOCK_SIZE;
    printInt(cacheSize);

    if (cacheSize == 1)
        printString(" block\n");
    else
        printString(" blocks\n");

    printString("Number of slabs in cache: ");
    printInt(this->numOfSlabs);
    printString("\n");

    printString("Number of objects in one slab: ");
    printInt(this->objNum);
    printString("\n");

    int freeCount = 0;
    int allocatedCount = 0;

    Slab* curr = this->slabsFree;
    objectCount(curr, freeCount, allocatedCount);

    curr = this->slabsPartial;
    objectCount(curr, freeCount, allocatedCount);

    curr = this->slabsFull;
    objectCount(curr, freeCount, allocatedCount);

    int percentage = (1000 * allocatedCount) / freeCount;
    percentage /= 10;
    printInt(percentage);

    printString("% of cache is used.\n");
}

void Cache::objectCount(Slab* slab, int &free, int &allocated) {
    Slab* curr = slab;
    while (curr) {
        free += curr->numOfSlots;
        allocated += (curr->numOfSlots - curr->numOfFreeSlots);
        curr = curr->next;
    }
}
