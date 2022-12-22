#include "../h/slab.h"
#include "../h/buddy.h"
#include "../h/CachePool.h"

void kmem_init(void* space, int block_num) {

    if (Buddy::buddyInit(space, block_num) == -1)
        return; // Exception

    CachePool::CachePoolInit();
        
}