#ifndef MMU_H
#define MMU_H

#include "../lib/hw.h"

static const uint8 PAGE_ORDER = 12;
static const uint64 PAGE_SIZE = 1 << 12;

class MMU {

private:
    MMU() = default;

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

    static int levelTwoCounter; // increment when you fill new entry in level two table
    static int levelThreeCounter;// increment when you fill new entry in level three table
    static int levelTwoCursor;
    static int levelThreeCursor;

    static uint64 entry;

    static void mapKernelSpace(uint64*);
    static void mapDev(uint64 start, uint64 end, EntryBits bits);
    static void zeroInit(uint64* addr, size_t n);

public:
    static uint64* rootTablePointer;
    static void MMUInit();
    static void updateEntry(uint64 vaddr);


};


#endif // MMU_H