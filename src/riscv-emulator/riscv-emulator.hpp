#pragma once

#include "../linux-emulator/linux-emulator.hpp"
#include "../mmu/mmu.hpp"
#include <cstdint>

class RiscvEmulator
{
  public:
    RiscvEmulator(Mmu &mmu) : mmu(mmu), registers(), linux_emulator(mmu) {}

    void run(uint32_t entry_point);

  private:
    uint32_t fetch_instruction() const;

    void execute_instruction(uint32_t inst);

    enum class RegisterName
    {
        zero, // x0 zero Hard-wired zero
        ra,   // x1 ra Return address
        sp,   // x2 sp Stack pointer
        gp,   // x3 gp Global pointer
        tp,   // x4 tp Thread pointer
        t0,   // x5 t0 Temporary/alternate link register
        t1,   // x6 t1 Temporary
        t2,   // x7 t2 Temporary
        s0,   // x8 s0/fp Saved register/frame pointer
        s1,   // x9 s1 Saved register
        a0,   // x10 a0 Function argument/return value
        a1,   // x11 a1 Function argument/return value
        a2,   // x12 a2 Function argument
        a3,   // x13 a3 Function argument
        a4,   // x14 a4 Function argument
        a5,   // x15 a5 Function argument
        a6,   // x16 a6 Function argument
        a7,   // x17 a7 Function argument
        s2,   // x18 s2 Saved register
        s3,   // x19 s3 Saved register
        s4,   // x20 s4 Saved register
        s5,   // x21 s5 Saved register
        s6,   // x22 s6 Saved register
        s7,   // x23 s7 Saved register
        s8,   // x24 s8 Saved register
        s9,   // x25 s9 Saved register
        s10,  // x26 s10 Saved register
        s11,  // x27 s11 Saved register
        t3,   // x28 t3 Temporary
        t4,   // x29 t4 Temporary
        t5,   // x30 t5Temporary
        t6    // x31 t6 Temporary
    };

    void set_pc(uint32_t virt_addr)
    {
        assert(virt_addr < mmu.size());
        registers[32] = virt_addr;
    }

    void set_register(uint8_t index, uint32_t value)
    {
        assert(index > 0 && index <= 32);
        std::cerr << "Setting register x" << std::dec << (uint16_t)index << " = " << value << '\n';
        registers[index] = value;
    }

    void set_register(RegisterName reg, uint32_t value)
    {
        set_register((int8_t)reg, value);
    }

    uint32_t get_register(uint8_t index) const
    {
        assert(index >= 0 && index <= 32);
        return registers[index];
    }

    uint32_t get_register(RegisterName reg) const
    {
        return get_register((uint8_t)reg);
    }

    uint32_t get_pc() const
    {
        return registers[32];
    }

    void next_pc()
    {
        registers[32] += sizeof(uint32_t);
    }

  private:
    bool skip_pc_update = false;
    Mmu &mmu;
    uint32_t registers[33];
    LinuxEmulator linux_emulator;
    bool running = true;
};