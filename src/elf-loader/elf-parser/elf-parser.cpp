
#include <bitset>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>

#include "elf-parser.hpp"

std::vector<Segment> ElfParser::parse_segments()
{
    const uint8_t *file_begin = file_data.data();

    ElfHeader elf_header = parse_elf_header();

    Elf32_Phdr *program_header_table = (Elf32_Phdr *)(file_begin + elf_header.program_header_offset);

    std::vector<Segment> segments;
    for (uint16_t i = 0; i < elf_header.num_program_headers; ++i)
    {
        Elf32_Phdr *program_header = program_header_table + i;
        segments.push_back(Segment{
            .type = program_header->p_type,
            .file_offset = program_header->p_offset,
            .file_size = program_header->p_filesz,
            .virtual_address = program_header->p_vaddr,
            .mem_size = program_header->p_memsz,
            .align = program_header->p_align});
    }

    return segments;
}

ElfHeader ElfParser::parse_elf_header()
{
    const uint8_t *file_begin = file_data.data();
    const Elf32_Ehdr *elf_header = (Elf32_Ehdr *)file_begin;

    return ElfHeader{
        .entry_point = elf_header->e_entry,
        .architecture = elf_header->e_machine,
        .num_program_headers = elf_header->e_phnum,
        .program_header_offset = elf_header->e_phoff};
}