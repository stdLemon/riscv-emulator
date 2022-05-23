#include "mmu.hpp"
#include <cstring>
#include <iostream>

void Mmu::write_from(uint32_t virt_addr, const uint8_t *begin, const uint8_t *end)
{
    auto write_size = end - begin;
    std::cerr << "write [" << virt_addr << "; " << virt_addr + write_size << ')' << std::endl;
    assert(virt_addr + write_size < memory.size());
    memcpy(memory.data() + virt_addr, begin, write_size);
}

void Mmu::read_bunch(uint32_t virt_addr, uint8_t *out_buf, uint32_t size)
{
    assert(virt_addr + size < memory.size());
    std::cerr << "read [" << virt_addr << "; " << virt_addr + size << ')' << std::endl;
    memcpy(out_buf, memory.data() + virt_addr, size);
}

void Mmu::set(uint32_t virt_addr, uint8_t value, uint32_t size)
{
    assert(virt_addr + size < memory.size());
    memory.insert(memory.begin() + virt_addr, value, size);
}

uint32_t Mmu::allocate(uint32_t size, uint32_t alloc_addr)
{
    if (alloc_addr == 0)
    {
        alloc_addr = brk_alloc;
    }

    if (alloc_addr < brk_alloc)
    {
        return 0;
    }

    if (alloc_addr + size > memory.size())
    {
        return 0;
    }

    if (first_alloc == 0)
    {
        first_alloc = alloc_addr;
    }

    brk_alloc = alloc_addr + size;

    std::cerr << "allocation addr " << std::hex << alloc_addr << " size " << std::hex << size << std::endl;
    return alloc_addr;
}