#include "../h/slab.h"
#include "../h/buddy.h"
#include "../h/cache.h"

void
kmem_init(void* space, int block_num)
{
    if (Buddy::buddyInit(space, block_num) == -1)
        return; // Exception

    CachePool::CachePoolInit();
    // TODO - inicijalizovati Slab (m.m. bafere itd.)
}

kmem_cache_t*
kmem_cache_create(const char* name, size_t size, void (*ctor)(void*), void (*dtor)(void*))
{
    return new Cache(name, size, ctor, dtor);
}

int
kmem_cache_shrink(kmem_cache_t* cachep)
{
    return cachep->cacheShrink();
}

void*
kmem_cache_alloc(kmem_cache_t* cachep)
{
    return cachep->cacheAlloc();
}

void
kmem_cache_free(kmem_cache_t* cachep, void* objp)
{
    cachep->cacheFree(objp);
}


void
kmem_cache_destroy(kmem_cache_t* cachep)
{
    delete cachep;
}

