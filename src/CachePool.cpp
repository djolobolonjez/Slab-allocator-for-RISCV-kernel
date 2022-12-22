#include "../h/CachePool.h"
#include "../h/buddy.h"
#include "../h/system.h"
#include "../h/cache.h"

CachePool::CacheEvidence* CachePool::head = nullptr;
CachePool::CacheEvidence* CachePool::tail = nullptr;
Cache* CachePool::cacheHead = nullptr;
Cache* CachePool::cacheTail = nullptr;

void CachePool::CachePoolInit() {
    head = tail = createEvidence();

    tail->next = head->next;
    tail->numOfSlots = head->numOfSlots;
    tail->slots = head->slots;
    tail->freeSlot = head->freeSlot;
}

CachePool::CacheEvidence* CachePool::createEvidence() {

    CacheEvidence* newEvidence = (CacheEvidence*) Buddy::alloc(0);
    newEvidence->next = nullptr;
    newEvidence->numOfSlots = 0;
    while (newEvidence->numOfSlots * sizeof(Cache) + sizeof(CacheEvidence) <= BLOCK_SIZE)
        newEvidence->numOfSlots++;

    newEvidence->numOfSlots--;

    newEvidence->slots = (Cache*) ((char*)newEvidence + sizeof(CacheEvidence));
    newEvidence->freeSlot = newEvidence->slots;

    for (int i = 0; i < newEvidence->numOfSlots - 1; i++)
        *(Cache**)(&newEvidence->slots[i]) = &newEvidence->slots[i + 1];
    *(Cache**)(&newEvidence->slots[newEvidence->numOfSlots - 1]) = nullptr;

    return newEvidence;

}

Cache* CachePool::allocateSlot() {

    Cache* ret = nullptr;
    CacheEvidence* curr = head;

    for (; curr; curr = curr->next)
        if (curr->freeSlot) break;

    if (!curr)
        curr = tail = tail->next = createEvidence();

    // TODO - obraditi greske ako nema dovoljno memorije!

    ret = curr->freeSlot;
    curr->freeSlot = *(Cache**)curr->freeSlot;

    if (!cacheTail) {
        cacheHead = cacheTail = ret;
        cacheHead->next = cacheTail->next = nullptr;
        cacheHead->prev = cacheTail->prev = nullptr;
    }
    else {
        ret->prev = cacheTail;
        ret->next = nullptr;
        cacheTail = cacheTail->next = ret;
    }

    return ret;
}
