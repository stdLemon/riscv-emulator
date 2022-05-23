#pragma once

#include "../mmu/mmu.hpp"
#include "syscall.hpp"
#include <cstdint>

class LinuxEmulator
{
  public:
    LinuxEmulator(Mmu &mmu) : mmu(mmu) {}
  
    std::pair<uint32_t, bool> handle_syscall(const Syscall &syscall);

    int32_t handle_read(uint32_t fd, uint32_t buff_addr, uint32_t size);

    int32_t handle_write(uint32_t fd, uint32_t buff_addr, uint32_t size);

    int32_t handle_fstat(uint32_t fd, uint32_t stat_out);

    int32_t handle_brk(uint32_t addr);

  private:
    Mmu &mmu;
};