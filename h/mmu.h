#ifndef MMU_H
#define MMU_H

#include "../lib/hw.h"

static const uint8 PAGE_ORDER = 12;
static const uint64 PAGE_SIZE = 1 << 12;

#define PAGE_ALIGN(VADDR) (uint64*) ((VADDR >> 10UL) << 12UL)

class MMU {

private:
    MMU() = default;

    enum MMU_FLAGS {
        PAGE_FAULT,
        PAGE_UNMAP
    };

    enum EntryBits {
        Valid = 1 << 0,
        Read = 1 << 1,
        Write = 1 << 2,
        Execute = 1 << 3,
        User = 1 << 4,
        Global = 1 << 5,
        Access = 1 << 6,
        Dirty = 1 << 7,

        ReadWrite = Read | Write,
        ReadExecute = Read | Execute,
        ReadWriteExecute = Read | Write | Execute,

        UserReadWrite = Read | Write | User,
        UserReadExecute = Read | Execute | User,
        UserReadWriteExecute = Read | Write | Execute | User,
    };
    
    static uint64* kspbegin;
    static uint64* kspend;
    static uint64* wrapbegin;
    static uint64* wrapend;
    static uint64* ubegin;
    static uint64* uend;
    
    static void map(uint64 vaddr, EntryBits bits);

    static void pmap(uint64 start, uint64 end, EntryBits bits);
    static void punmap(uint64 start, uint64 end);

    static void invalid(uint64 vaddr, MMU_FLAGS flags);
    static void zeroInit(uint64* addr, size_t n);

    friend class Riscv;
    friend class MemoryAllocator;
public:
    static uint64* rootTablePointer;
    static void MMUInit();
    static bool kspace(uint64 vaddr);
    static bool ufetch(uint64 vaddr);
    
    static void MMUFinalize();
};


#endif // MMU_H
