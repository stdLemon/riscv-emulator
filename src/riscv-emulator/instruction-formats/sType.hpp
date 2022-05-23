#pragma once

#include <bitset>
#include <cstdint>
#include <ostream>

struct Stype
{
    int16_t imm : 12;
    uint8_t rs2 : 5;
    uint8_t rs1 : 5;
    uint8_t func3 : 3;

    static Stype from(uint32_t inst)
    {
        uint8_t imm_115 = (inst >> 25) & 0b1111111;
        uint8_t imm_40 = (inst >> 7) & 0b11111;
        int16_t imm = (imm_115 << 5) | imm_40;
        
        return Stype
        {
            .imm = imm,
            .rs2 = (uint8_t)((inst >> 20) & 0b11111),
            .rs1 = (uint8_t)((inst >> 15) & 0b11111),
            .func3 = (uint8_t)((inst >> 12) & 0b111)
        };
    }

    friend std::ostream &operator<<(std::ostream &out, const Stype &s_type)
    {
        out << "imm = " << std::dec << (uint16_t)s_type.imm << ' '
            << "rs1 = " << std::dec << (uint16_t)s_type.rs1 << ' '
            << "rs2 = " << std::dec << (uint16_t)s_type.rs2 << ' '
            << "func3 = " << std::bitset<3>(s_type.func3);

        return out;
    }
};