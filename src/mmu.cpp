#include "../h/mmu.h"
#include "../h/buddy.h"

uint64* MMU::rootTablePointer = nullptr;
uint64 MMU::entry = 0;
int MMU::levelTwoCounter = 0;
int MMU::levelThreeCounter = 0;
int MMU::levelTwoCursor = 0;
int MMU::levelThreeCursor = 0;

void MMU::MMUInit() {

    rootTablePointer = (uint64*) Buddy::alloc(0);
    zeroInit(rootTablePointer, PAGE_SIZE / sizeof(uint64));

    uint64* levelTwo = (uint64*) Buddy::alloc(0);
    zeroInit(levelTwo, PAGE_SIZE / sizeof(uint64));

    uint64* levelThree = (uint64*) Buddy::alloc(0);
    zeroInit(levelThree, PAGE_SIZE / sizeof(uint64));

    uint64 vaddr = (uint64) levelTwo;
    uint64 levelTwoEntry = (vaddr >> 30) & 0x1ffUL;

    rootTablePointer[levelTwoEntry] = (((uint64)levelTwo >> 2) | Valid);

    vaddr = (uint64) levelThree;
    uint64 levelThreeEntry = (vaddr >> 21) & 0x1ffUL;

    levelTwo[levelThreeEntry] = (((uint64)levelThree >> 2) | Valid);

    levelTwoCounter++;
    mapKernelSpace(levelThree);
    mapDev(0x10000000, 0x10000100, EntryBits(Write | Read)); // map UART
    mapDev(0x0c000000, 0x0c002001, EntryBits(Write | Read));  // map PLIC
    mapDev(0x0c200000, 0x0c208001, EntryBits(Write | Read));
}

void MMU::mapKernelSpace(uint64* levelThree) {
    entry = 0x80000000UL;
    uint64* currTable = levelThree;

    size_t numOfPages = ((uint64)Buddy::KERNEL_END_ADDR - entry) / PAGE_SIZE - 1;
    for (size_t i = 0; i <= numOfPages; i++) {
        uint64 vaddr = entry + i * PAGE_SIZE;
        uint64 pgDesc = (vaddr >> 2) | Valid | ReadWriteExecute;

        uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};
        if (levelThreeCounter == 0) {
            uint64* tableTwo = (uint64*) ((rootTablePointer[vpn[2]] >> 10) << 12);
            tableTwo[vpn[1]] = ((uint64) currTable >> 2) | Valid;
        }

        currTable[vpn[0]] = pgDesc;

        if (++levelThreeCounter == (PAGE_SIZE / 8)) {
            levelThreeCounter = 0;
            levelTwoCounter++;
            currTable = (uint64*) Buddy::alloc(0);
            zeroInit(currTable, PAGE_SIZE / sizeof(uint64));
        }
    }
}

void MMU::updateEntry(uint64 vaddr) {
    vaddr &= ~(PAGE_SIZE - 1);
    uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};
    if (!(rootTablePointer[vpn[2]] & Valid)) {
        uint64* levelTwo = (uint64*) Buddy::alloc(0);
        zeroInit(levelTwo, PAGE_SIZE / sizeof(uint64));

        uint64 levelTwoEntry = ((uint64)levelTwo >> 2) | Valid;
        rootTablePointer[vpn[2]] = levelTwoEntry;

        uint64* levelThree = (uint64*) Buddy::alloc(0);
        zeroInit(levelThree, PAGE_SIZE / sizeof(uint64));

        levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;

        uint64 pgDesc = (vaddr >> 2) | Valid | ReadWriteExecute;
        levelThree[vpn[0]] = pgDesc;
    }
    else {
        uint64* tableTwo = (uint64*) ((rootTablePointer[vpn[2]] >> 10) << 12);
        uint64* tableThree = (uint64*) ((tableTwo[vpn[1]] >> 10) << 12);
        tableThree[vpn[0]] = (vaddr >> 2) | Valid | ReadWriteExecute;
    }
}

void MMU::mapDev(uint64 start, uint64 end, EntryBits bits) {
    end &= ~(PAGE_SIZE - 1);

    size_t pageNum = (end - start) / PAGE_SIZE;
    for (size_t i = 0; i <= pageNum; i++) {
        uint64 vaddr = start + i * PAGE_SIZE;
        uint64 pgDesc = (vaddr >> 2) | Valid | bits;

        uint64 vpn[] = {(vaddr >> 12) & 0x1ffUL, (vaddr >> 21) & 0x1ffUL, (vaddr >> 30) & 0x1ffUL};
        if (!(rootTablePointer[vpn[2]] & Valid)) {
            uint64* levelTwo = (uint64*) Buddy::alloc(0);
            zeroInit(levelTwo, PAGE_SIZE / sizeof(uint64));

            uint64 levelTwoEntry = ((uint64)levelTwo >> 2) | Valid;
            rootTablePointer[vpn[2]] = levelTwoEntry;

            uint64* levelThree = (uint64*) Buddy::alloc(0);
            zeroInit(levelThree, PAGE_SIZE / sizeof(uint64));

            levelTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;

            uint64* tableThree = (uint64*) ((levelTwo[vpn[1]] >> 10) << 12);
            tableThree[vpn[0]] = pgDesc;
        }
        else {
            uint64* tableTwo = (uint64*) ((rootTablePointer[vpn[2]] >> 10) << 12);
            if (!(tableTwo[vpn[1]] & Valid)) {
                uint64* levelThree = (uint64*) Buddy::alloc(0);
                zeroInit(levelThree, PAGE_SIZE / sizeof(uint64));

                tableTwo[vpn[1]] = ((uint64) levelThree >> 2) | Valid;
            }
            uint64* tableThree = (uint64*) ((tableTwo[vpn[1]] >> 10) << 12);
            tableThree[vpn[0]] = pgDesc;

        }
    }
}

void MMU::zeroInit(uint64 *addr, size_t n) {
    for (size_t i = 0; i < n; i++)
        addr[i] = 0;
}


