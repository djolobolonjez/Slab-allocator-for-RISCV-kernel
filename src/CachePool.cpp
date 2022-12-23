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

    CacheRecord* newRecord = (CacheRecord*) Buddy::alloc(0);
    if (!newRecord) return nullptr; // No available memory

    newRecord->next = nullptr;
    newRecord->numOfSlots = 0;
    while (newRecord->numOfSlots * sizeof(Cache) + sizeof(CacheRecord) <= BLOCK_SIZE)
        newRecord->numOfSlots++;

    newRecord->numOfSlots--;
    newRecord->numOfFreeSlots = newRecord->numOfSlots;

    newRecord->slots = (Cache*) ((char*)newRecord + sizeof(CacheRecord));
    newRecord->freeSlot = newRecord->slots;

    for (int i = 0; i < newRecord->numOfSlots - 1; i++)
        *(Cache**)(&newRecord->slots[i]) = &newRecord->slots[i + 1];
    *(Cache**)(&newRecord->slots[newRecord->numOfSlots - 1]) = nullptr;

    return newRecord;

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

    // TODO - Free all slabs before deallocating cache itself

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