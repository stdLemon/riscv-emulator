#pragma once

#include <cstdint>
#include <ostream>

struct Utype
{
    uint32_t imm : 20;
    uint8_t rd : 5;

    static Utype from(uint32_t inst)
    {

        return Utype
        {
            .imm = (inst & ~0xFFF) >> 12,
            .rd = (uint8_t)((inst >> 7) & 0b11111)
        };
    }

    friend std::ostream &operator<<(std::ostream &out, const Utype &u_type)
    {
        out << "rd = " << std::dec << (uint16_t)u_type.rd << ' '
            << "imm = 0x" << std::hex << u_type.imm;

        return out;
    }
};