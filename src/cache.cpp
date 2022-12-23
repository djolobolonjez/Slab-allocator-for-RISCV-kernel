#include "../h/cache.h"
#include "../h/system.h"

Cache::Cache(const char *name, size_t size, void (*ctor)(void *), void (*dtor)(void *)) {
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
    this->slotSize = size;
    this->ctor = ctor;
    this->dtor = dtor;
    this->shrink = 1;

    this->slabOrder = estimateOrder(this->slotSize);
    this->slabSize = (1 << this->slabOrder) * BLOCK_SIZE;
    this->objNum = getNumberOfObjects(this->slabSize, this->slotSize);

    this->numOfSlabs = 0;

    this->slabsFull = nullptr;
    this->slabsPartial = nullptr;
    this->slabsFree = nullptr;
}

void* Cache::operator new(size_t size) {
    return CachePool::allocateSlot();
}

void Cache::operator delete(void *addr)  {
    CachePool::deallocateSlot((Cache*)addr);
}

size_t Cache::estimateOrder(size_t slotSize) {
    size_t slabOrder;
    if (slotSize > BLOCK_SIZE)
        slabOrder = slotSize / BLOCK_SIZE;
    else
        slabOrder = 0;

    return slabOrder;
}

size_t Cache::getNumberOfObjects(size_t slabSize, size_t slotSize) {
    int numObj = 0;
    while (numObj * slotSize + numObj * sizeof(unsigned) + sizeof (SlabAllocator::Slab) <= slabSize)
        numObj++;

    return numObj - 1;
}

void Cache::setEmptyToPartial(SlabAllocator::Slab* slab) {

}

void* Cache::cacheAlloc() {

    if (this->slabsPartial == nullptr && this->slabsFree == nullptr)
        SlabAllocator::createSlab(this->slabOrder, this);
    else {
        if (this->slabsPartial == nullptr) {}
            //setEmptyToPartial();
    }
}
