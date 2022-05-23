#pragma once

#include <bitset>
#include <cstdint>
#include <ostream>

struct Rtype
{
    uint8_t func7 : 7;
    uint8_t rs2 : 5;
    uint8_t rs1 : 5;
    uint8_t func3 : 3;
    uint8_t rd : 5;

    static Rtype from(uint32_t inst)
    {
        return Rtype
        {
            .func7 = (uint8_t)((inst >> 25) & 0b1111111),
            .rs2 = (uint8_t)((inst >> 20) & 0b11111),
            .rs1 = (uint8_t)((inst >> 15) & 0b11111),
            .func3 = (uint8_t)((inst >> 12) & 0b111),
            .rd = (uint8_t)((inst >> 7) & 0b11111)
        };
    }

    friend std::ostream &operator<<(std::ostream &out, const Rtype &r_type)
    {
        out << "rd = " << std::dec << (uint16_t)r_type.rd << ' '
            << "rs1 = " << std::dec << (uint16_t)r_type.rs1 << ' '
            << "rs2 = " << std::dec << (uint16_t)r_type.rs2 << ' '
            << "func3 = " << std::bitset<3>(r_type.func3) << ' '
            << "func7 = " << std::bitset<7>(r_type.func7);

        return out;
    }
};