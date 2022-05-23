#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../mmu/mmu.hpp"
#include "elf-parser/elf-parser.hpp"

class ElfLoader
{
  public:
    ElfLoader(Mmu &mmu) : mmu(mmu) {}

    uint32_t load(const std::string &file_path);

  private:
    std::vector<uint8_t> load_file(const std::string &file_path);

  private:
    Mmu &mmu;
};