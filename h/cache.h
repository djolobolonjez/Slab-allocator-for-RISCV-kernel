#ifndef CACHE_H
#define CACHE_H

#include "../lib/hw.h"
#include "../h/CachePool.h"

class Slab;

class Cache {
public:
    enum SlabGroup {
        FULL,
        PARTIAL,
        FREE
    };
private:
    const char* name;
    size_t slotSize;
    void (*ctor)(void*);
    void (*dtor)(void*);

    Cache* next, *prev;
    Slab* slabsFull, *slabsPartial, *slabsFree;

    int shrink;
    size_t slabSize;
    size_t slabOrder;
    size_t objNum; // number of objects in one slab

    size_t numOfSlabs;

    static size_t estimateOrder(size_t slotSize);
    static size_t getNumberOfObjects(size_t slabSize, size_t slotSize);
    static void deallocSlabGroup(Slab* slab);
    static void objectCount(Slab* slab, int& free, int& allocated);

    friend class CachePool;
    friend class Slab;
public:
    Cache(const char* name, size_t size, void (*ctor)(void*), void (*dtor)(void*));
    ~Cache();

    void setShrink(int _shrink) { this->shrink = _shrink; }

    void* cacheAlloc();
    void cacheFree(void* objp);
    int cacheShrink();
    void printInfo();

    static void destroyCache(Cache* cachep);

    void moveFree(Slab* slab, SlabGroup grp);
    void moveSlab(Slab* slab, SlabGroup grp);

    void* operator new(size_t size);
    void operator delete(void* addr);

};


#endif // CACHE_H