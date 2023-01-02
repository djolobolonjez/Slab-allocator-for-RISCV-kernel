#include "../../h/mmu.h"
#include "../../h/buddy.h"
#include "../../h/system.h"

uint64* MMU::rootTablePointer = nullptr;

void MMU::MMUInit() {
    rootTablePointer = (uint64*) Buddy::alloc(0);
    zeroInit(rootTablePointer, PAGE_SIZE / sizeof(uint64));

    uint64* user_code_start = (uint64*)&USER_CODE_START;
    uint64* user_code_end = (uint64*)&USER_CODE_END;

    pmap((uint64)user_code_start, (uint64)user_code_end, UserReadWriteExecute); // map user code section
    pmap(0x80000000, (uint64)Buddy::KERNEL_END_ADDR, ReadWriteExecute); // map kernel space
    pmap(0x10000000, 0x10000100, ReadWrite); // map UART
    pmap(0x0c000000, 0x0c002001, ReadWrite);  // map PLIC
    pmap(0x0c200000, 0x0c208001, ReadWrite);
}

void MMU::pmap(uint64 start, uint64 end, EntryBits bits) {
    start &= ~(PAGE_SIZE - 1);
    end &= ~(PAGE_SIZE - 1);

    size_t pageNum = (end - start) / PAGE_SIZE;

    uint64* levelTwo, *levelThree;

    for (size_t i = 0; i <= pageNum; i++) {
        uint64 vaddr = start + i * PAGE_SIZE;
        uint64 pgDesc = (vaddr >> 2) | Valid | bits;

        uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};
        if (!(rootTablePointer[vpn[2]] & Valid)) {
            levelTwo = (uint64*) Buddy::alloc(0);
            zeroInit(levelTwo, PAGE_SIZE / sizeof(uint64));

            uint64 levelTwoEntry = ((uint64)levelTwo >> 2) | Valid;
            rootTablePointer[vpn[2]] = levelTwoEntry;

            levelThree = (uint64*) Buddy::alloc(0);
            zeroInit(levelThree, PAGE_SIZE / sizeof(uint64));

            levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;
        }
        else {
            levelTwo = (uint64*) ((rootTablePointer[vpn[2]] >> 10) << 12);
            if (!(levelTwo[vpn[1]] & Valid)) {
                levelThree = (uint64*) Buddy::alloc(0);
                zeroInit(levelThree, PAGE_SIZE / sizeof(uint64));

                levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;
            }
            else
                levelThree = (uint64*) ((levelTwo[vpn[1]] >> 10) << 12);
        }
        if (!(levelThree[vpn[0]] & Valid))
            levelThree[vpn[0]] = pgDesc;
    }
}

void MMU::invalid(uint64 vaddr, MMU_FLAGS flags) {
    vaddr &= ~(PAGE_SIZE - 1);
    uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};
    uint64* levelTwo, *levelThree;

    if (!(rootTablePointer[vpn[2]] & Valid))
        return;

    levelTwo = (uint64*)((rootTablePointer[vpn[2]] >> 10) << 12);
    if (!(levelTwo[vpn[1]] & Valid))
        return;

    levelThree = (uint64*)((levelTwo[vpn[1]] >> 10) << 12);
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
    if ((uint64*)vaddr >= Buddy::KERNEL_START_ADDR && (uint64*)vaddr < Buddy::KERNEL_END_ADDR)
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

