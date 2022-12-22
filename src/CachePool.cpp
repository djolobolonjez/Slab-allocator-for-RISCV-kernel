#include "../h/CachePool.h"
#include "../h/buddy.h"
#include "../h/system.h"
#include "../h/cache.h"

CachePool::CacheRecord* CachePool::head = nullptr;
CachePool::CacheRecord* CachePool::tail = nullptr;
Cache* CachePool::cacheHead = nullptr;
Cache* CachePool::cacheTail = nullptr;

void CachePool::CachePoolInit() {
    head = tail = createRecord();

    tail->next = head->next;
    tail->numOfSlots = head->numOfSlots;
    tail->slots = head->slots;
    tail->freeSlot = head->freeSlot;
}

CachePool::CacheRecord* CachePool::createRecord() {

    CacheRecord* newEvidence = (CacheRecord*) Buddy::alloc(0);
    if (!newEvidence) return nullptr; // No available memory

    newEvidence->next = nullptr;
    newEvidence->numOfSlots = 0;
    while (newEvidence->numOfSlots * sizeof(Cache) + sizeof(CacheRecord) <= BLOCK_SIZE)
        newEvidence->numOfSlots++;

    newEvidence->numOfSlots--;
    newEvidence->numOfFreeSlots = newEvidence->numOfSlots;

    newEvidence->slots = (Cache*) ((char*)newEvidence + sizeof(CacheRecord));
    newEvidence->freeSlot = newEvidence->slots;

    for (int i = 0; i < newEvidence->numOfSlots - 1; i++)
        *(Cache**)(&newEvidence->slots[i]) = &newEvidence->slots[i + 1];
    *(Cache**)(&newEvidence->slots[newEvidence->numOfSlots - 1]) = nullptr;

    return newEvidence;

}

Cache* CachePool::allocateSlot() {

    Cache* ret = nullptr;
    CacheRecord* curr = head;

    for (; curr; curr = curr->next)
        if (curr->freeSlot) break;

    if (!curr) {
        curr = createRecord();
        if (!tail) head = tail = curr;
        else tail = tail->next = curr;
    }

    if (!curr)
        return nullptr; // No memory

    ret = curr->freeSlot;
    ret->myRecord = curr;

    curr->freeSlot = *(Cache**)curr->freeSlot;
    curr->numOfFreeSlots--;

    return ret;
}

void CachePool::deallocateSlot(Cache *handle) {

    if (handle->prev == nullptr) {
        cacheHead = cacheHead->next;
        if (!cacheHead) cacheTail = nullptr;
        else cacheHead->prev = nullptr;
    }
    else {
        if (handle->next == nullptr) {
            cacheTail = cacheTail->prev;
            cacheTail->next = nullptr;
        }
        else {
            handle->prev->next = handle->next;
            handle->next->prev = handle->prev;
        }
    }
    handle->next = handle->prev = nullptr;

    CacheRecord* record = handle->myRecord;
    *(Cache**)handle = record->freeSlot;
    record->freeSlot = handle;

    record->numOfFreeSlots++;
    if (record->numOfFreeSlots == record->numOfSlots && record != head)
        destroyRecord(record);

}

void CachePool::destroyRecord(CacheRecord* record) {

    CacheRecord* curr = head, *prev = nullptr;
    while (curr != record) {
        prev = curr;
        curr = curr->next;
    }

    prev->next = curr->next;
    if (curr == tail) tail = prev;
    curr->next = nullptr;

    Buddy::free(curr, 0);
}