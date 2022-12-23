#ifndef CACHE_H
#define CACHE_H

#include "../lib/hw.h"
#include "../h/CachePool.h"
#include "../h/SlabAllocator.h"

class Cache {
private:
    const char* name;
    size_t slotSize;
    void (*ctor)(void*);
    void (*dtor)(void*);

    Cache* next, *prev;
    SlabAllocator::Slab* slabsFull, *slabsPartial, *slabsFree;

    CachePool::CacheRecord* myRecord;

    int shrink;
    size_t slabSize;
    size_t slabOrder;
    size_t objNum; // number of objects in one slab

    size_t numOfSlabs;

    static size_t estimateOrder(size_t slotSize);
    static size_t getNumberOfObjects(size_t slabSize, size_t slotSize);

    friend class CachePool;
    friend class SlabAllocator;
public:
    Cache(const char* name, size_t size, void (*ctor)(void*), void (*dtor)(void*));

    void* cacheAlloc();
    void setEmptyToPartial(SlabAllocator::Slab* slab);

    void* operator new(size_t size);
    void operator delete(void* addr);
};


#endif // CACHE_H