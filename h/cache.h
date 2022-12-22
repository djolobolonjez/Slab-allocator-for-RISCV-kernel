#ifndef CACHE_H
#define CACHE_H

#include "../lib/hw.h"
#include "../h/CachePool.h"

class Slab;

class Cache {
private:
    const char* name;
    size_t objSize;
    void (*ctor)(void*);
    void (*dtor)(void*);

    Cache* next, *prev;
    Slab* slabsFull, *slabsPartial, *slabsFree;

    CachePool::CacheRecord* myRecord;

    int shrink;
    int slabSize;
    int objNum; // number of objects in one slab

    friend class CachePool;
public:
    Cache(const char* name, size_t size,
          void (*ctor)(void*), void (*dtor)(void*)) {

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
        this->objSize = size;
        this->ctor = ctor;
        this->dtor = dtor;
        this->shrink = 1;
    }

    void* operator new(size_t size);
    void operator delete(void* addr);
};


#endif // CACHE_H