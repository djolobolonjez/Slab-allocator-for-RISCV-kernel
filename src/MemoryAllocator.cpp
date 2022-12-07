#include "../h/MemoryAllocator.h"

MemoryAllocator::BlockHeader* MemoryAllocator::fmem_head = nullptr;
int MemoryAllocator::init = 0;

int MemoryAllocator::tryToJoin(BlockHeader* curr) {

    if(!curr) return 0;
    if(curr->next && (char*)curr + curr->size + sizeof(BlockHeader) == (char*)curr->next){
        curr->size += (curr->next->size + sizeof(BlockHeader));
        curr->next = curr->next->next;
        if(curr->next) curr->next->prev = curr;
        return 1;
    }
    else return 0;
}

void* MemoryAllocator::kmem_alloc(size_t size){

    if(MemoryAllocator::fmem_head == nullptr && !MemoryAllocator::init){
        MemoryAllocator::fmem_head = (BlockHeader*)HEAP_START_ADDR;
        MemoryAllocator::fmem_head->next = nullptr;
        MemoryAllocator::fmem_head->prev = nullptr;
        MemoryAllocator::fmem_head->size = (char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR - sizeof(BlockHeader);
        MemoryAllocator::init = 1;
    }

    BlockHeader* curr = MemoryAllocator::fmem_head;
    while(curr){
        if(curr->size >= size*MEM_BLOCK_SIZE) break;
        curr = curr->next;
    }

    if(!curr) return nullptr;

    size_t remainder = curr->size - size*MEM_BLOCK_SIZE;
    if(remainder > sizeof(BlockHeader)) {
        curr->size = size * MEM_BLOCK_SIZE;
        size_t offset = sizeof(BlockHeader) + size * MEM_BLOCK_SIZE;
        BlockHeader* newBlock = (BlockHeader*) ((char*) curr + offset);
        if(curr->prev){
            curr->prev->next = newBlock;
            newBlock->prev = curr->prev;
        }
        else{
            newBlock->prev = nullptr;
            MemoryAllocator::fmem_head = newBlock;
        }
        newBlock->next = curr->next;
        if(newBlock->next) newBlock->next->prev = newBlock;
        newBlock->size = remainder - sizeof(BlockHeader);

        if((char*)newBlock >= (char*)HEAP_END_ADDR || (char*)newBlock + newBlock->size + sizeof(BlockHeader) > (char*)HEAP_END_ADDR)
            MemoryAllocator::fmem_head = nullptr;

    }
    else{
        if(curr->prev){
            curr->prev->next = curr->next;
            if(curr->next) curr->next->prev = curr->prev;

        }
        else{
            if(curr->next) curr->next->prev = nullptr;
            MemoryAllocator::fmem_head = curr->next;
        }
    }
    curr->next = nullptr;
    curr->prev = nullptr;

    return (char*)curr + sizeof(BlockHeader);

}

int MemoryAllocator::kmem_free(void* addr){

    if(!addr) return -1;

    BlockHeader* curr = nullptr;

    if(!MemoryAllocator::fmem_head || (char*)addr < (char*)MemoryAllocator::fmem_head)
        curr = nullptr;
    else{
        curr = MemoryAllocator::fmem_head;
        while(curr->next &&  (char*)addr > (char*)curr->next)
            curr = curr->next;
    }

    BlockHeader* newBlock = (BlockHeader*)((char*)addr - sizeof(BlockHeader));
    newBlock->prev = curr;
    if(curr) newBlock->next = curr->next;
    else newBlock->next = MemoryAllocator::fmem_head;
    if(newBlock->next) newBlock->next->prev = newBlock;
    if(curr) curr->next = newBlock;
    else MemoryAllocator::fmem_head = newBlock;

    tryToJoin(newBlock);
    tryToJoin(curr);

    return 0;

}