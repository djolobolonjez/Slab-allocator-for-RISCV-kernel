#include "../../h/MemoryAllocator.h"
#include "../../h/mmu.h"
#include "../../h/system.h"

MemoryAllocator::BlockHeader* MemoryAllocator::fmem_head = nullptr;
int MemoryAllocator::init = 0;

int MemoryAllocator::tryToJoin(BlockHeader* curr) {

    if(!curr) return 0;
    if(curr->next && (uint8*)curr + curr->size + sizeof(BlockHeader) == (uint8*)curr->next){
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
        MemoryAllocator::fmem_head->size = (uint8*)HEAP_END_ADDR - (uint8*)HEAP_START_ADDR - sizeof(BlockHeader);
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
        BlockHeader* newBlock = (BlockHeader*) ((uint8*) curr + offset);
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

        if((uint8*)newBlock >= (uint8*)HEAP_END_ADDR || (uint8*)newBlock + newBlock->size + sizeof(BlockHeader) > (uint8*)HEAP_END_ADDR)
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

    return (uint8*)curr + sizeof(BlockHeader);

}

int MemoryAllocator::kmem_free(void* addr){

    if(!addr) return -1;

    BlockHeader* curr = nullptr;

    if(!MemoryAllocator::fmem_head || (uint8*)addr < (uint8*)MemoryAllocator::fmem_head)
        curr = nullptr;
    else{
        curr = MemoryAllocator::fmem_head;
        while(curr->next &&  (uint8*)addr > (uint8*)curr->next)
            curr = curr->next;
    }

    BlockHeader* newBlock = (BlockHeader*)((uint8*)addr - sizeof(BlockHeader));
    newBlock->prev = curr;
    if(curr) newBlock->next = curr->next;
    else newBlock->next = MemoryAllocator::fmem_head;
    if(newBlock->next) newBlock->next->prev = newBlock;
    if(curr) curr->next = newBlock;
    else MemoryAllocator::fmem_head = newBlock;

    tryToUnmap(newBlock);

    if (tryToJoin(newBlock) > 0)
        tryToUnmap(newBlock);

    if (tryToJoin(curr) > 0)
        tryToUnmap(curr);

    return 0;
}

void MemoryAllocator::tryToUnmap(BlockHeader *addr) {
    uint64 start = (uint64) addr;

    size_t offset = addr->size;
    uint64 end = (uint64)((uint8*)addr + offset);

    if (start % BLOCK_SIZE != 0) {
        start &= ~(BLOCK_SIZE - 1);
        start += BLOCK_SIZE;
    }

    if (end % BLOCK_SIZE != 0) {
        end &= ~(BLOCK_SIZE - 1);
    }

    if (end <= start) return; // Cannot free table entry

    MMU::punmap(start, end);
}
