#pragma once

#include <bitset>
#include <cstdint>
#include <ostream>

struct Jtype
{
    int32_t imm : 20;
    uint8_t rd : 5;

    static Jtype from(uint32_t inst)
    {
        uint8_t imm20 = (inst >> 31) & 1;
        uint16_t imm101 = (inst >> 21) & 0b1111111111;
        uint8_t imm11 = (inst >> 20) & 1;
        uint8_t imm1912 = (inst >> 12) & 0b11111111;

        return Jtype
        {
            .imm = (imm20 << 20) | 
            (imm1912 << 12) | 
            (imm11 << 11) | 
            (imm101 << 1),
            
            .rd = (uint8_t)((inst >> 7) & 0b11111)
        };
    }

    friend std::ostream &operator<<(std::ostream &out, const Jtype &i_type)
    {
        out << "rd = " << std::dec << (uint16_t)i_type.rd << ' '
            << "imm = 0x" << std::dec << i_type.imm;

        return out;
    }
};