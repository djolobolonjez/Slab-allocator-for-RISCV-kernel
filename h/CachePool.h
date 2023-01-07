#ifndef CACHE_POOL_H
#define CACHE_POOL_H

#include "system.h"

class Cache;

class CachePool {

private:
    struct CacheRecord {
        int numOfSlots;
        int numOfFreeSlots;
        CacheRecord* next;

        Cache* freeSlot;
        Cache* slots;
    };

    static CacheRecord* head, *tail;
    static Cache* cacheHead, *cacheTail;

    static Cache* memoryBuffers[BUFFER_NUM];
    static Cache* memBuffSlabs[BUFFER_NUM];

    static CacheRecord* createRecord();
    static void destroyRecord(CacheRecord* record);

    friend class Cache;
public:
    static void CachePoolInit();
    static Cache* allocateSlot();
    static void deallocateSlot(Cache* handle);
    static void SlabInit();

    static void* allocateBuffer(size_t size);
    static void deallocateBuffer(const void* objp);
    
    static void CachePoolFinalize();
    static void SlabFinalize();

    static Cache* getSlabCache(int index) { return memBuffSlabs[index]; }
    static Cache* getBufferCache(int index) { return memoryBuffers[index]; }

    static void bufferInit(int index, size_t size);
    static size_t getOrder(size_t x);

    static int getPowerOfTwo(size_t x);
    static size_t powerOfTwoSize(size_t x);
};

#endif // CACHE_POOL_H
