#ifndef CACHE_POOL_H
#define CACHE_POOL_H

#include "../h/system.h"

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

    static CacheRecord* createRecord();
    static void destroyRecord(CacheRecord* record);

    static int getPowerOfTwo(size_t x);

    friend class Cache;
public:
    static void CachePoolInit();
    static Cache* allocateSlot();
    static void deallocateSlot(Cache* handle);
    static void SlabInit();

    static void* allocateBuffer(size_t size);
    static void deallocateBuffer(const void* objp);

    static size_t powerOfTwoSize(size_t x);
};

#endif // CACHE_POOL_H