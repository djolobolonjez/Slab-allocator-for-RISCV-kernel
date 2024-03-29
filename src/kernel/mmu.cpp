#include "../../h/mmu.h"
#include "../../h/buddy.h"
#include "../../h/system.h"
#include "../../h/tcb.h"

uint64* MMU::rootTablePointer = nullptr;
uint64 MMU::kspbegin = 0;
uint64 MMU::kspend = 0;
uint64 MMU::wrapbegin = 0;
uint64 MMU::wrapend = 0;
uint64 MMU::user_begin = 0;
uint64 MMU::user_end = 0;

void MMU::MMUInit() {
    rootTablePointer = pgalloc(0);

    sectionMap();

    uint64 user_data_begin = (uint64)&UDATA_BEGIN;
    uint64 user_data_end = (uint64)&UDATA_END;
    
    uint64 kcode_begin = (uint64)&KCODE_BEGIN;
    uint64 kcode_end = (uint64)&KCODE_END;

    pmap(user_begin, user_end, UserReadWriteExecute); // map user code section
    pmap(user_data_begin, user_data_end, UserReadWriteExecute); // map user data section

    // map kernel space
    pmap((uint64)Buddy::KERNEL_START_ADDR, (uint64)Buddy::KERNEL_END_ADDR - 1, ReadWriteExecute);
    pmap(kcode_begin, kcode_end, ReadWriteExecute);
    pmap(kspbegin, kspend, ReadWriteExecute);
    
    // map devices
    pmap(0x10000000, 0x10000100, ReadWrite); // map UART
    pmap(0x0c000000, 0x0c002001, ReadWrite);  // map PLIC
    pmap(0x0c200000, 0x0c208001, ReadWrite);
}

void MMU::pmap(uint64 start, uint64 end, EntryBits bits) {
    start &= ~(PAGE_SIZE - 1);
    end &= ~(PAGE_SIZE - 1);

    size_t pageNum = (end - start) / PAGE_SIZE;

    for (size_t i = 0; i <= pageNum; i++) {
        uint64 vaddr = start + i * PAGE_SIZE;
        map(vaddr, bits);   
    }
}

void MMU::map(uint64 vaddr, EntryBits bits) {

    vaddr &= ~(PAGE_SIZE - 1);
    uint64* levelTwo, *levelThree;
    
    uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};

    uint64 pgDesc = (vaddr >> 2) | Valid | bits;
    
    if (!(rootTablePointer[vpn[2]] & Valid)) {
            levelTwo = pgalloc(0);

            uint64 levelTwoEntry = ((uint64)levelTwo >> 2) | Valid;
            rootTablePointer[vpn[2]] = levelTwoEntry;

            levelThree = pgalloc(0);

            levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;
        }
        else {
            levelTwo = PAGE_ALIGN(rootTablePointer[vpn[2]]);
            if (!(levelTwo[vpn[1]] & Valid)) {

                levelThree = pgalloc(0);
                levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;
            }
            else
                levelThree = PAGE_ALIGN(levelTwo[vpn[1]]);
        }

    levelThree[vpn[0]] = pgDesc;
}

void MMU::invalid(uint64 vaddr, MMU_FLAGS flags) {
    vaddr &= ~(PAGE_SIZE - 1);
    uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};
    uint64* levelTwo, *levelThree;

    if (!(rootTablePointer[vpn[2]] & Valid))
        return;

    levelTwo = PAGE_ALIGN(rootTablePointer[vpn[2]]);
    if (!(levelTwo[vpn[1]] & Valid))
        return;

    levelThree = PAGE_ALIGN(levelTwo[vpn[1]]);
    uint64 pgDesc = 0;
    levelThree[vpn[0]] = pgDesc;

    if (flags == PAGE_UNMAP) {
        size_t entryNum = PAGE_SIZE / sizeof(uint64);
        for (size_t i = 0; i < entryNum; i++)
            if (levelThree[i] != 0)
                return;

        levelTwo[vpn[1]] = pgDesc;
        Buddy::free(levelThree, 0);

        for (size_t i = 0; i < entryNum; i++)
            if (levelTwo[i] != 0)
                return;

        rootTablePointer[vpn[2]] = pgDesc;
        Buddy::free(levelTwo, 0);
    }
}

void MMU::zeroInit(uint64 *addr, size_t n) {
    for (size_t i = 0; i < n; i++)
        addr[i] = 0;
}

bool MMU::kspace(uint64 vaddr) {
    if ((vaddr >= (uint64)Buddy::KERNEL_START_ADDR && vaddr < (uint64)Buddy::KERNEL_END_ADDR)
    	|| (vaddr >= kspbegin && vaddr < kspend))
        return true;
    return false;
}

void MMU::punmap(uint64 start, uint64 end) {
    uint64 mask = ~(PAGE_SIZE - 1);
    start &= mask, end &= mask;

    size_t pageNum = (end - start) / PAGE_SIZE;
    for (unsigned i = 0; i <= pageNum; i++) {
        invalid(start, PAGE_UNMAP);
        start += PAGE_SIZE;
    }
}

void MMU::MMUFinalize() {
    uint64* pmtp = rootTablePointer;
    uint64 descNum = PAGE_SIZE / sizeof(uint64);
    uint64 i;

    for (i = 0; i < descNum; i++) {
        if (pmtp[i] & Valid) {
            uint64* levelTwo = PAGE_ALIGN(pmtp[i]);
            uint64 ii;

            for (ii = 0; ii < descNum; ii++) {
                if (levelTwo[ii] & Valid) {
                    uint64* levelThree = PAGE_ALIGN(levelTwo[ii]);
                    Buddy::free(levelThree, 0);
                }
            }
            Buddy::free(levelTwo, 0);
        }
    }

    Buddy::free(pmtp, 0);
}

bool MMU::ufetch(uint64 vaddr) {
    if ((vaddr >= MMU::wrapbegin && vaddr < MMU::wrapend)
        || (vaddr >= MMU::user_begin && vaddr <= MMU::user_end)) {
        return true;
    }
    return false;
}

void MMU::sectionMap() {
    wrapbegin = (uint64)&WRAP_START;
    wrapend = (uint64)&WRAP_END;

    kspbegin = (uint64)&KDATA_BEGIN;
    kspend = (uint64)&KDATA_END;

    user_begin = (uint64)&USER_CODE_START;
    user_end = (uint64)&USER_CODE_END;
}

uint64* MMU::pgalloc(int order) {
    uint64* table = (uint64*) Buddy::alloc(order);

    size_t num = PAGE_SIZE / sizeof(uint64);
    zeroInit(table, num);

    return table;
}
