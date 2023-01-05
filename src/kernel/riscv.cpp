#include "../../h/MemoryAllocator.h"
#include "../../h/riscv.h"
#include "../../h/tcb.h"
#include "../../h/kernelsem.h"
#include "../../h/sleeping.h"
#include "../../h/kernelcons.h"
#include "../../h/printing.hpp"
#include "../../h/mmu.h"

void Riscv::sppUser(void (*fn)(void*), void* arg) {
    uint64 ra;
    asm volatile ("mv %[ra], ra" : [ra] "=r"(ra));

    Riscv::mc_sstatus(SSTATUS_SPP);
    asm volatile("csrw sepc, %[function]" : : [function] "r"(fn));
    asm volatile ("mv a0, %[arg]" : : [arg] "r"(arg));

    asm volatile ("mv ra, %[ra]" : : [ra] "r"(ra));
    asm volatile("sret");
}

void Riscv::sppKernel(void (*fn)(void*), void* arg) {
    uint64 ra;
    asm volatile ("mv %[ra], ra" : [ra] "=r"(ra));

    Riscv::ms_sstatus(SSTATUS_SPP);

    asm volatile("csrw sepc, %[function]" : : [function] "r"(fn));
    asm volatile ("mv a0, %[arg]" : : [arg] "r"(arg));

    asm volatile ("mv ra, %[ra]" : : [ra] "r"(ra));
    asm volatile("sret");
}

uint64 Riscv::r_sstatus() {
    uint64 volatile sstatus;
    asm volatile("csrr %[sstatus], sstatus" : [sstatus] "=r"(sstatus));
    return sstatus;
}

void Riscv::w_sstatus(uint64 sstatus) {

    asm volatile("csrw sstatus, %[sstatus]" : : [sstatus] "r"(sstatus));

}

uint64 Riscv::r_scause() {

    uint64 volatile scause;
    asm volatile("csrr %[scause], scause" : [scause] "=r"(scause));
    return scause;
}

void Riscv::w_cause(uint64 scause) {

    asm volatile("csrw scause, %[scause]" : : [scause] "r"(scause));
}

uint64 Riscv::r_stvec() {

    uint64 volatile stvec;
    asm volatile("csrr %[stvec], stvec" : [stvec] "=r"(stvec));
    return stvec;
}

void Riscv::w_stvec(uint64 stvec){

    asm volatile ("csrw stvec, %[stvec]" : : [stvec] "r"(stvec));
}

uint64 Riscv::r_stval() {
    uint64 volatile stval;
    asm volatile("csrr %[stval], stval" : [stval] "=r"(stval));
    return stval;
}

uint64 Riscv::r_sepc(){

    uint64 volatile sepc;
    asm volatile("csrr %[sepc], sepc" : [sepc] "=r"(sepc));
    return sepc;
}

void Riscv::w_sepc(uint64 sepc) {

    asm volatile("csrw sepc, %[sepc]" : : [sepc] "r"(sepc));
}

void Riscv::ms_sstatus(uint64 mask) {

    asm volatile("csrs sstatus, %[mask]" : : [mask] "r"(mask));
}

void Riscv::mc_sstatus(uint64 mask) {

    asm volatile("csrc sstatus, %[mask]" : : [mask] "r"(mask));
}

void Riscv::ms_sip(uint64 mask) {

    asm volatile("csrs sip, %[mask]" : : [mask] "r"(mask));
}

void Riscv::mc_sip(uint64 mask) {

    asm volatile("csrc sip, %[mask]" : : [mask] "r"(mask));
}

void Riscv::trapHandler()  {

    volatile uint64 number;
    asm volatile("mv %[value], a0" : [value] "=r"(number));

    uint64 cause = Riscv::r_scause();

    if(cause == 0x09 || cause == 0x08){

        volatile uint64 sepc = r_sepc() + 4;
        volatile uint64 sstatus = r_sstatus();

        if(number == MEM_ALLOC){
            uint64 alloc_arg;
            asm volatile("mv %[arg], a1" : [arg] "=r"(alloc_arg));
            MemoryAllocator::kmem_alloc(alloc_arg);
            W_RET
        }

        else if(number == MEM_FREE){
            void* free_arg;
            asm volatile("mv %[arg], a1" : [arg] "=r"(free_arg));
            MemoryAllocator::kmem_free(free_arg);
            W_RET
        }
        else if(number == THREAD_CREATE){

            thread_t* handle;
            void (*start_routine)(void*);
            void* arg;
            void* stck;

            ABI_REG_THREE(handle, start_routine, arg);
            LOAD_STACK(stck);

            TCB::createThread(handle, start_routine, arg, stck, 0, true);
            W_RET
        }
        else if(number == THREAD_EXIT){
            TCB::suspend();
            W_RET
        }
        else if(number == THREAD_DISPATCH){
            volatile uint64 sepc = r_sepc() + 4;
            volatile uint64 sstatus = r_sstatus();
            TCB::timeSliceCounter = 0;
            TCB::yield();
            w_sstatus(sstatus);
            w_sepc(sepc);
        }
        else if(number == TCB_CREATE){
            thread_t* handle;
            void (*start_routine)(void*);
            void* arg;
            void* stck;

            ABI_REG_THREE(handle, start_routine, arg);
            LOAD_STACK(stck);

            TCB::createThread(handle, start_routine, arg, stck, 1, false);
            W_RET
        }
        else if (number == THREAD_START) {
            thread_t* handle;

            asm volatile ("mv %[handle], a1" : [handle] "=r"(handle));
            TCB::startThread(handle);
            W_RET
        }
        else if(number == SEM_OPEN){
            sem_t* handle;
            unsigned init;

            asm volatile("mv %[handle], a1" : [handle] "=r"(handle));
            asm volatile("mv %[init], a2" : [init] "=r"(init));

            KernelSem::createSem(handle, init);
            W_RET
        }
        else if(number == SEM_CLOSE){
            sem_t handle;

            asm volatile("mv %[handle], a1" : [handle] "=r"(handle));

            KernelSem::deleteSem(handle);
            W_RET
        }
        else if(number == SEM_WAIT){
            sem_t id;

            asm volatile("mv %[id], a1" : [id] "=r"(id));
            id->wait();

            W_RET
        }
        else if(number == SEM_SIGNAL){
            sem_t id;

            asm volatile("mv %[id], a1" : [id] "=r"(id));
            id->signal();

            W_RET
        }
        else if(number == TIME_SLEEP){
            time_t time;
            asm volatile("mv %[time], a1" : [time] "=r"(time));

            TCB::running->setAsleep(true);
            Sleeping::add(time);
            TCB::dispatch();
        }
        else if(number == PUTC){
            char c;
            asm volatile("mv %[ch], a1" : [ch] "=r"(c));

            KernelConsole::getInstance()->put(c);
        }
        else if(number == GETC){
            KernelConsole::getInstance()->get();
            W_RET
        }

        w_sstatus(sstatus);
        w_sepc(sepc);
    }
   
    else if(cause == 0x8000000000000001UL){
        volatile uint64 sepc = r_sepc();
        volatile uint64 sstatus = r_sstatus();

        TCB::timeSliceCounter++;
        Riscv::mc_sip(SIP_SSIP);

        Sleeping::remove();
        if(TCB::timeSliceCounter >= TCB::running->getTimeSlice()) {
            TCB::timeSliceCounter = 0;
            TCB::yield();
        }
        w_sstatus(sstatus);
        w_sepc(sepc);
    }
    else if(cause == 0x8000000000000009UL){
        volatile uint64 sepc = r_sepc();
        volatile uint64 sstatus = r_sstatus();

        int irq = plic_claim();
        if(irq == CONSOLE_IRQ){
            KernelConsole* cons  = KernelConsole::getInstance();
            cons->uartReceive->signal();
            cons->uartTransmit->signal();
        }

        plic_complete(irq);

        w_sstatus(sstatus);
        w_sepc(sepc);
    }

    else if(cause == 12) {
        uint64 vaddr = Riscv::r_stval();
        volatile uint64 sstatus = r_sstatus();

        MMU::invalid(vaddr, MMU::PAGE_FAULT);

        if (sstatus & SSTATUS_SPP)
            MMU::pmap(vaddr, vaddr, MMU::ReadWriteExecute);
        else
            MMU::pmap(vaddr, vaddr, MMU::UserReadWriteExecute);
    }
    else if (cause == 13 || cause == 15) {
        uint64 vaddr = Riscv::r_stval();
        volatile uint64 status = Riscv::r_sstatus();

        MMU::invalid(vaddr, MMU::PAGE_FAULT);

        if (MMU::kspace(vaddr) && !(status & SSTATUS_SPP)) {
            printString("Access violation!");
        }
        else {
            MMU::pmap(vaddr, vaddr, MMU::UserReadWriteExecute);
        }
    }
    else {
        printString("scause: ");
        printInt(cause);
        printString("\n");
    }
}
