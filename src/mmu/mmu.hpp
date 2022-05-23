#pragma once

#include <assert.h>
#include <cstdint>
#include <iostream>
#include <vector>

class Mmu
{
  public:
    Mmu(uint32_t size) : memory(size, 0) {}

    template <typename T>
    void write(uint32_t virt_addr, T value)
    {
        assert(virt_addr + sizeof(T) < memory.size());
        std::cerr << "write " << virt_addr << " = " << (int)value << '\n';
        *(T *)(memory.data() + virt_addr) = value;
    }

    template <typename T>
    T read(uint32_t virt_addr)
    {
        assert(virt_addr + sizeof(T) < memory.size());
        T value = *(T *)(memory.data() + virt_addr);
        std::cerr << "read " << virt_addr << " = " << (int)value << '\n';
        return value;
    }

    void write_from(uint32_t virt_addr, const uint8_t *begin, const uint8_t *end);

    void read_bunch(uint32_t virt_addr, uint8_t *out_buf, uint32_t size);

    void set(uint32_t virt_addr, uint8_t value, uint32_t size);

    uint32_t allocate(uint32_t size, uint32_t alloc_addr = 0);

    uint32_t size() const
    {
        return memory.size();
    }

    uint32_t get_first_alloc() const
    {
        return first_alloc;
    }

    uint32_t get_brk_alloc() const
    {
        return brk_alloc;
    }

  private:
    std::vector<uint8_t> memory;
    uint32_t first_alloc = 0;
    uint32_t brk_alloc = 0;
};