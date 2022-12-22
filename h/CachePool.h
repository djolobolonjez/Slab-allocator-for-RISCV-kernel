#ifndef CACHE_POOL_H
#define CACHE_POOL_H

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

    static CacheRecord* createRecord();
    static void destroyRecord(CacheRecord* record);

    friend class Cache;
public:
    static void CachePoolInit();
    static Cache* allocateSlot();
    static void deallocateSlot(Cache* handle);
};

#endif // CACHE_POOL_H