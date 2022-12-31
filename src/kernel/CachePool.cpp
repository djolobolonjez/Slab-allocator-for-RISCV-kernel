#include "../../h/CachePool.h"
#include "../../h/buddy.h"
#include "../../h/cache.h"
#include "../../h/SlabAllocator.h"

Cache* CachePool::memoryBuffers[BUFFER_NUM] = {nullptr};

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

    newRecord->slots = (Cache*) ((uint8*)newRecord + sizeof(CacheRecord));
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

    uint64 mask = ~0UL << 12;
    CacheRecord* record = (CacheRecord*) ((uint64)handle & mask);

    *(Cache**)handle = record->freeSlot;
    record->freeSlot = handle;

    record->numOfFreeSlots++;
    if (record->numOfFreeSlots == record->numOfSlots && record != head)
        destroyRecord(record);

}

void CachePool::destroyRecord(CacheRecord* record) {

    CacheRecord* curr = head, *prev = nullptr;

    while (curr != record)
        prev = curr, curr = curr->next;

    prev->next = curr->next;
    if (curr == tail) tail = prev;
    curr->next = nullptr;

    Buddy::free(curr, 0);
}

void CachePool::SlabInit() {

    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        memoryBuffers[i] = new Cache("size-", 1 << (MIN_BUFF_ORDER + i), nullptr, nullptr);
        memoryBuffers[i]->setGroup(Cache::SMALL_MEMORY_BUFFER);
    }
}

int CachePool::getPowerOfTwo(size_t x) {
    size_t value = 1;
    int order = 0;
    while (x > value) {
        value <<= 1;
        order++;
    }
    return order - MIN_BUFF_ORDER;
}

void* CachePool::allocateBuffer(size_t size) {
    if (size < MIN_BUFF_SIZE || size > MAX_BUFF_SIZE)
        return nullptr; // Exception

    int index = getPowerOfTwo(size);
    return memoryBuffers[index]->cacheAlloc();
}

void CachePool::deallocateBuffer(const void *objp) {
    if (objp == nullptr) return;
    Slab::putObject(const_cast<void*>(objp));
}

size_t CachePool::powerOfTwoSize(size_t x) {
    size_t value = 1;
    while (x > value)
        value <<= 1;

    return value;
}