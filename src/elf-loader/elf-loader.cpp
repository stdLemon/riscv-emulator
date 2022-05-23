#include <cstdint>
#include <fstream>
#include <iostream>

#include "elf-loader.hpp"

uint32_t ElfLoader::load(const std::string &file_path)
{
    auto file_data = load_file(file_path);
    ElfParser elf_parser(file_data);

    auto elf_header = elf_parser.parse_elf_header();
    std::cout << "Elf Header\n"
              << elf_header << '\n';

    if (elf_header.architecture != EM_RISCV)
    {
        return 0;
    }

    std::cout << "Loading segments\n";
    auto segments = elf_parser.parse_segments();
    for (const Segment &segment : segments)
    {
        if (segment.type == PT_LOAD)
        {
            std::cout << segment << '\n';

            const uint8_t *segment_begin = file_data.data() + segment.file_offset;
            const uint8_t *segment_end = segment_begin + segment.file_size;
            uint32_t remainder = segment.mem_size % segment.align;
            uint32_t aligned_size = segment.mem_size + segment.align - remainder;

            mmu.allocate(aligned_size, segment.virtual_address);
            mmu.write_from(segment.virtual_address, segment_begin, segment_end);
        }
    }

    return elf_header.entry_point;
}

std::vector<uint8_t> ElfLoader::load_file(const std::string &file_path)
{
    std::vector<uint8_t> bytes;
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(file_path, std::ios::binary);
    file.seekg(0, std::ios_base::end);
    std::streampos file_size = file.tellg();
    bytes.resize(file_size);
    file.seekg(0, std::ios_base::beg);

    file.read((char *)&bytes[0], file_size);

    return bytes;
}