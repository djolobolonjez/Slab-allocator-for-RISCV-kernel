#ifndef CACHE_H
#define CACHE_H

#include "../lib/hw.h"

class Slab;

class Cache {
private:
    const char* name;
    size_t objSize;
    void (*ctor)(void*);
    void (*dtor)(void*);

    Cache* next, *prev;
    Slab* slabsFull, *slabsPartial, *slabsFree;

    int shrink;
    int slabSize;
    int objNum; // number of objects in one slab

    friend class CachePool;
public:
    int flag = 0;

};


#endif // CACHE_H