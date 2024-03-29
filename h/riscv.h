#ifndef riscv_h
#define riscv_h

#include "../lib/hw.h"

#define SSTATUS_SPP (1L << 8) // previous privilege
#define SSTATUS_SPIE (1L << 5) // previous intr enable
#define SSTATUS_SIE (1L << 1)  // intr enable

#define SSTATUS_SUM (1L << 18)
#define SSTATUS_MXR (1l << 19)

#define SIP_SSIP (1L << 1)
#define SIP_STIP (1l << 5)
#define SIP_SEIP (1L << 9)

#define enable_interrupts Riscv::w_sstatus(Riscv::r_sstatus() | SSTATUS_SIE);
#define disable_interrupts Riscv::w_sstatus(Riscv::r_sstatus() | ~SSTATUS_SIE);

class Riscv{

public:
    static void sppKernel(void (*fn)(void*), void* arg);

    static void sppUser(void (*fn)(void*), void* arg);

    // push x3..x31 on stack
    static void pushRegisters();

    // pop x3..x31 from stack
    static void popRegisters();

    // read from sstatus register
    static uint64 r_sstatus();

    // write in sstatus register
    static void w_sstatus(uint64 sstatus);

    // read from scause register
    static uint64 r_scause();

    // write in scause register
    static void  w_cause(uint64 scause);

    // read from stvec register
    static uint64 r_stvec();

    // write in stvec register
    static void w_stvec(uint64 stvec);

    static uint64 r_stval();

    // read from register sepc
    static uint64 r_sepc();

    // write in sepc register
    static void w_sepc(uint64 sepc);

    // mask set sstatus register
    static void ms_sstatus(uint64 mask);

    // mask clear sstatus register
    static void mc_sstatus(uint64 mask);

    // mask set sip register
    static void ms_sip(uint64 mask);

    // mask clear sip register
    static void mc_sip(uint64 mask);

    // supervisor trap
    static void supervisorTrap();

private:
    static void trapHandler();

};

#endif // riscv_h
