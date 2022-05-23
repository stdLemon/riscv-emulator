#include <bitset>
#include <cstdint>
#include <ostream>

struct Segment
{
    uint32_t type;
    uint32_t file_offset;
    uint32_t file_size;
    uint32_t virtual_address;
    uint32_t mem_size;
    uint32_t align;

    friend std::ostream &operator<<(std::ostream &out, const Segment &segment)
    {
        out << "File offset = 0x" << std::hex << segment.file_offset << ' '
            << "Virtual address = 0x" << std::hex << segment.virtual_address << ' '
            << "File size = 0x" << std::hex << segment.file_size << ' '
            << "Memory size = 0x" << std::hex << segment.mem_size << ' ';

        return out;
    }
};