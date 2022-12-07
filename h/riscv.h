#ifndef _riscv_h
#define _riscv_h

#include "../lib/hw.h"

#define SSTATUS_SPP (1L << 8) // previous privilege
#define SSTATUS_SPIE (1L << 5) // previous intr enable
#define SSTATUS_SIE (1L << 1)  // intr enable

#define SIP_SSIP (1L << 1)
#define SIP_STIP (1l << 5)
#define SIP_SEIP (1L << 9)

#define enable_interrupts Riscv::w_sstatus(Riscv::r_sstatus() | SSTATUS_SIE);
#define disable_interrupts Riscv::w_sstatus(Riscv::r_sstatus() | ~SSTATUS_SIE);

class Riscv{

public:

    static void sppKernel();

    static void sppUser();

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
    static void trapHandler(/*uint64 a0, uint64 a1, uint64 a2, uint64 a3, uint64 a4*/);

};

#endif
