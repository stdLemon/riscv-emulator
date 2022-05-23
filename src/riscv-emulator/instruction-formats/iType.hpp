#pragma once

#include <bitset>
#include <cstdint>
#include <ostream>

struct Itype
{
    int16_t imm : 12;
    uint8_t rs1 : 5;
    uint8_t func3 : 3;
    uint8_t rd : 5;

    static Itype from(uint32_t inst)
    {
        return Itype
        {
            .imm = (int16_t)(inst >> 20),
            .rs1 = (uint8_t)((inst >> 15) & 0b11111),
            .func3 = (uint8_t)((inst >> 12) & 0b111),
            .rd = (uint8_t)((inst >> 7) & 0b11111)
        };
    }

    friend std::ostream &operator<<(std::ostream &out, const Itype &i_type)
    {
        out << "rd = " << std::dec << (uint16_t)i_type.rd << ' '
            << "imm = 0x" << std::dec << i_type.imm << ' '
            << "rs1 = " << std::dec << (uint16_t)i_type.rs1 << ' '
            << "func3 = " << std::bitset<7>(i_type.func3);

        return out;
    }
};