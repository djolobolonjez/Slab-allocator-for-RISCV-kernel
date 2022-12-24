#include "../h/cache.h"
#include "../h/system.h"
#include "../h/SlabAllocator.h"

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
    while (numObj * slotSize + numObj * sizeof(unsigned) + sizeof (Slab) <= slabSize)
        numObj++;

    return numObj - 1;
}

void Cache::moveFree(Slab* slab, SlabGroup grp) {

    this->slabsFree = this->slabsFree->next;
    if (this->slabsFree) this->slabsFree->prev = nullptr;
    slab->prev = nullptr;

    Slab* header = (grp == Cache::PARTIAL ? this->slabsPartial : this->slabsFull);

    slab->group = grp;
    slab->next = header;
    if (header) header->prev = slab;
    if (grp == Cache::PARTIAL)
        this->slabsPartial = slab;
    else
        this->slabsFull = slab;
}

void Cache::moveSlab(Slab *slab, SlabGroup grp) {

    Slab* curr = (slab->group == Cache::PARTIAL ? this->slabsPartial : this->slabsFull);
    Slab* header = nullptr;
    if (slab->group == Cache::PARTIAL)
        header = (grp == Cache::FREE ? this->slabsFree : this->slabsFull);
    else
        header = (grp == Cache::PARTIAL ? this->slabsPartial : this->slabsFree);

    if (slab->prev == nullptr) {
        curr = curr->next;
        if (curr) curr->prev = nullptr;
        if (slab->group == Cache::PARTIAL)
            this->slabsPartial = curr;
        else
            this->slabsFull = curr;
    }
    else {
        slab->prev->next = slab->next;
        if (slab->next) slab->next->prev = slab->prev;
    }

    slab->group = grp;
    slab->prev = nullptr;
    slab->next = header;
    if (header) header->prev = slab;

    if (grp == Cache::FREE)
        this->slabsFree = slab;
    else if (grp == Cache::PARTIAL)
        this->slabsPartial = slab;
    else
        this->slabsFull = slab;
}

void* Cache::cacheAlloc() {

    size_t oldNumOfSlabs = this->numOfSlabs;

    if (this->slabsPartial == nullptr && this->slabsFree == nullptr)
        Slab::createSlab(this->slabOrder, this);
    else {
        if (this->slabsPartial == nullptr)
            moveFree(this->slabsFree, Cache::PARTIAL);
    }

    if (this->numOfSlabs > oldNumOfSlabs)
        setShrink(0); // istraziti shrinkovanje, da li se prvi prvoj alokaciji setuje ili ne??? mozda alocirati odma jedan slab i izbeci ovo? reset je prilikom shrinka

    void* objp = Slab::takeObject(this->slabsPartial);
    if (this->slabsPartial->numOfFreeSlots == 0) // proveriti slucaj kada ima vise partial slabova!!! tj. da li postoji taj slucaj??
        moveSlab(this->slabsPartial, Cache::FULL);

    return objp;
}

void Cache::cacheFree(void* objp) {
    Slab::putObject(objp);
    if (this->ctor)
        this->ctor(objp);
}