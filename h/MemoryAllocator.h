#ifndef _memory_allocator_cpp
#define _memory_allocator_cpp

#include "../lib/hw.h"

class MemoryAllocator{

private:

    MemoryAllocator() = default;

    // header for free segment list

    struct BlockHeader{
        BlockHeader* next = nullptr;
        BlockHeader* prev = nullptr;
        size_t size;
    };

    static int init;
    static BlockHeader* fmem_head;

    // memory coalescing
    static int tryToJoin(BlockHeader* curr);

public:

    // allocate memory on heap
    static void* kmem_alloc(size_t size);

    // deallocate memory
    static int kmem_free(void*);
};




#endif