#include "../../h/mmu.h"
#include "../../h/buddy.h"
#include "../../h/system.h"
#include "../../h/kprint.h"

uint64* MMU::rootTablePointer = nullptr;
uint64* MMU::kspbegin = nullptr;
uint64* MMU::kspend = nullptr;

void MMU::MMUInit() {
    rootTablePointer = (uint64*) Buddy::alloc(0);
    zeroInit(rootTablePointer, PAGE_SIZE / sizeof(uint64));

    uint64* user_code_start = (uint64*)&USER_CODE_START;
    uint64* user_code_end = (uint64*)&USER_CODE_END;
    
    uint64* udata_begin = (uint64*)&UDATA_BEGIN;
    uint64* udata_end = (uint64*)&UDATA_END;
    
    uint64* kcode_begin = (uint64*)&KCODE_BEGIN;
    uint64* kcode_end = (uint64*)&KCODE_END;
     
    kspbegin = (uint64*)&KDATA_BEGIN;
    kspend = (uint64*)&KDATA_END;

    pmap((uint64)user_code_start, (uint64)user_code_end, UserReadWriteExecute); // map user code section
    pmap((uint64)udata_begin, (uint64)udata_end, UserReadWriteExecute); // map user data section
    
    // map kernel space
    pmap((uint64)Buddy::KERNEL_START_ADDR, (uint64)Buddy::KERNEL_END_ADDR, ReadWriteExecute); 
    pmap((uint64)kcode_begin, (uint64)kcode_end, ReadWriteExecute);
    pmap((uint64)kspbegin, (uint64)kspend, ReadWriteExecute);
    
    // map devices
    pmap(0x10000000, 0x10000100, ReadWrite); // map UART
    pmap(0x0c000000, 0x0c002001, ReadWrite);  // map PLIC
    pmap(0x0c200000, 0x0c208001, ReadWrite);
}

void MMU::pmap(uint64 start, uint64 end, EntryBits bits) {
    start &= ~(PAGE_SIZE - 1);
    end &= ~(PAGE_SIZE - 1);

    size_t descNum = PAGE_SIZE / sizeof(uint64);
    size_t pageNum = (end - start) / PAGE_SIZE;

    uint64* levelTwo, *levelThree;

    for (size_t i = 0; i <= pageNum; i++) {
        uint64 vaddr = start + i * PAGE_SIZE;
        uint64 pgDesc = (vaddr >> 2) | Valid | bits;

        uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};
        if (!(rootTablePointer[vpn[2]] & Valid)) {
            levelTwo = (uint64*) Buddy::alloc(0);
            zeroInit(levelTwo, descNum);

            uint64 levelTwoEntry = ((uint64)levelTwo >> 2) | Valid;
            rootTablePointer[vpn[2]] = levelTwoEntry;

            levelThree = (uint64*) Buddy::alloc(0);
            zeroInit(levelThree, descNum);

            levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;
        }
        else {
            levelTwo = (uint64*) ((rootTablePointer[vpn[2]] >> 10) << 12);
            if (!(levelTwo[vpn[1]] & Valid)) {
                levelThree = (uint64*) Buddy::alloc(0);
                zeroInit(levelThree, descNum);

                levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;
            }
            else
                levelThree = (uint64*) ((levelTwo[vpn[1]] >> 10) << 12);
        }

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
    uint64* va = (uint64*)vaddr;
    if ((va >= Buddy::KERNEL_START_ADDR && va < Buddy::KERNEL_END_ADDR)
    	|| (va >= kspbegin && va < kspend))
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
            uint64* levelTwo = (uint64*)((pmtp[i] >> 10) << 12);
            uint64 ii;

            for (ii = 0; ii < descNum; ii++) {
                if (levelTwo[ii] & Valid) {
                    uint64* levelThree = (uint64*) ((levelTwo[ii] >> 10) << 12);
                    Buddy::free(levelThree, 0);
                }
            }
            Buddy::free(levelTwo, 0);
        }
    }

    Buddy::free(pmtp, 0);
}

void MMU::printPMT() {
    uint64* pmtp = rootTablePointer;
    uint64 descNum = PAGE_SIZE / sizeof(uint64);

    for (size_t i = 0; i < descNum; i++) {
        if (pmtp[i] & Valid) {
            kprintString("Level One Table:\n");
            kprintInt((uint64)PAGE_ALIGN(pmtp[i]), 16);
            kprintString("\n");
            uint64* levelTwo = PAGE_ALIGN(pmtp[i]);
            for (size_t j = 0; j < descNum; j++) {
                if (levelTwo[j] & Valid) {
                    kprintString("Level Two Table:\n");
                    kprintInt((uint64)PAGE_ALIGN(levelTwo[j]), 16);
                    kprintString("\n");
                    uint64* levelThree = PAGE_ALIGN(levelTwo[j]);
                    for (size_t k = 0; k < descNum; k++){
                        kprintString("Level Three Descriptor:\n");
                        kprintInt((uint64)PAGE_ALIGN(levelThree[k]), 16);
                        kprintString("\n");
                    }
                }
            }
        }
    }
}


