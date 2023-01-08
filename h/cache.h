#ifndef CACHE_H
#define CACHE_H

#include "../lib/hw.h"
#include "CachePool.h"

class Slab;

class Cache {

private:
    enum SlabGroup {
        FULL,
        PARTIAL,
        FREE
    };

    enum ObjectGroup {
        SMALL_MEMORY_BUFFER,
        KERNEL_OBJECT
    };

    const char* name;
    size_t slotSize;
    void (*ctor)(void*);
    void (*dtor)(void*);

    ObjectGroup group;

    Cache* next, *prev;
    Slab* slabsFull, *slabsPartial, *slabsFree;

    int shrink;
    size_t slabSize;
    size_t slabOrder;
    size_t objNum; // number of objects in one slab

    size_t numOfSlabs;

    int type;
    int error; // 0 - no errors

    static size_t getOrder(size_t slotSize, size_t& numSlots);
    static void deallocSlabGroup(Slab* slab);
    static void objectCount(Slab* slab, int& free, int& allocated);

    void setError(int err) { this->error = err; }

    friend class CachePool;
    friend class Slab;
public:
    Cache(const char* name, size_t size, void (*ctor)(void*), void (*dtor)(void*));
    ~Cache();

    void setGroup(ObjectGroup _group) { this->group = _group; }
    void setShrink(int _shrink) { this->shrink = _shrink; }
    void setType(int _type) { this->type = _type; }

    ObjectGroup getGroup() { return this->group; }

    void* cacheAlloc();
    void cacheFree(void* objp);
    int cacheShrink();

    void printInfo();
    int printErrorMessage() const;

    void slabAlloc();

    static void destroyCache(Cache* cachep);

    void moveFree(Slab* slab, SlabGroup grp);
    void moveSlab(Slab* slab, SlabGroup grp);

    void* operator new(size_t size);
    void operator delete(void* addr);
};


#endif // CACHE_H