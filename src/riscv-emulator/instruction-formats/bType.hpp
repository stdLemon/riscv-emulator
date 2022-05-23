#pragma once

#include <bitset>
#include <cstdint>
#include <ostream>

struct Btype
{
    int32_t imm : 20;
    uint8_t rs2 : 5;
    uint8_t rs1 : 5;
    uint8_t func3 : 3;

    static Btype from(uint32_t inst)
    {
        uint8_t imm12 = (inst >> 31) & 1;
        uint8_t imm105 = (inst >> 25) & 0b111111;
        uint8_t imm41 = (inst >> 8) & 0b1111;
        uint8_t imm11 = (inst >> 7) & 1;
        int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm105 << 5) | (imm41 << 1);

        return Btype
        {
            .imm = (imm << 19) >> 19,
            .rs2 = (uint8_t)((inst >> 20) & 0b11111),
            .rs1 = (uint8_t)((inst >> 15) & 0b11111),
            .func3 = (uint8_t)((inst >> 12) & 0b111)
        };
    }

    friend std::ostream &operator<<(std::ostream &out, const Btype &b_type)
    {
        out << "imm = " << std::dec << (uint16_t)b_type.imm << ' '
            << "rs1 = " << std::dec << (uint16_t)b_type.rs1 << ' '
            << "rs2 = " << std::dec << (uint16_t)b_type.rs2 << ' '
            << "func3 = " << std::bitset<3>(b_type.func3);

        return out;
    }
};