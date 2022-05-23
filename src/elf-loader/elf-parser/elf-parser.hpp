#pragma once

#include <elf.h>
#include <string>
#include <vector>

#include "elf-header.hpp"
#include "segment.hpp"

class ElfParser
{
  public:
    ElfParser(const std::vector<uint8_t> &file_data) : file_data(file_data) {}
    bool load_file();

    ElfHeader parse_elf_header();
    std::vector<Segment> parse_segments();

  private:
    std::vector<uint8_t> file_data;
};
