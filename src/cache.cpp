#include "../h/cache.h"

void* Cache::operator new(size_t size) {
    return CachePool::allocateSlot();
}

void Cache::operator delete(void *addr)  {
    CachePool::deallocateSlot((Cache*)addr);
}