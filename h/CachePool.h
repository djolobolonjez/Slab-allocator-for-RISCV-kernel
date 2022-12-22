#ifndef CACHE_POOL_H
#define CACHE_POOL_H

class Cache;

class CachePool {

private:
    struct CacheEvidence {
        int numOfSlots;
        CacheEvidence* next;
        Cache* freeSlot;
        Cache* slots;
    };

    static CacheEvidence* head, *tail;
    static Cache* cacheHead, *cacheTail;

    static CacheEvidence* createEvidence();
public:
    static void CachePoolInit();
    static Cache* allocateSlot();
};

#endif // CACHE_POOL_H