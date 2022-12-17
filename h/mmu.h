#ifndef _MMU_H
#define _MMU_H

#include "../lib/hw.h"

static const uint8 PAGE_ORDER = 12;
static const uint64 PAGE_SIZE = 1 << 12;

class MMU {
private:
    uint64* alignAddress(uint64* addr, uint64 order);

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
};




#endif // _MMU_H
