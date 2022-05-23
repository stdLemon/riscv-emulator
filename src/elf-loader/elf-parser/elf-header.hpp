#pragma once
#include <cstdint>
#include <ostream>

struct ElfHeader
{
    uint32_t entry_point;
    uint32_t architecture;
    uint32_t num_program_headers;
    uint32_t program_header_offset;

    friend std::ostream &operator<<(std::ostream &out, const ElfHeader &elf_header)
    {
        out << "Entry Point 0x" << std::hex << elf_header.entry_point << '\n';
        out << "Architecture " << std::dec << elf_header.architecture << '\n';
        out << "Number of program headers " << std::dec << elf_header.num_program_headers << '\n';
        out << "Program header offset " << std::dec << elf_header.program_header_offset << '\n';

        return out;
    }
};
