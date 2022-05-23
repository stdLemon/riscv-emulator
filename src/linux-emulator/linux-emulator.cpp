#include "linux-emulator.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <utility>

// https://github.com/riscv-collab/riscv-gnu-toolchain/blob/master/linux-headers/include/asm-generic/stat.h
struct stat
{
    unsigned long st_dev;  /* Device.  */
    unsigned long st_ino;  /* File serial number.  */
    unsigned int st_mode;  /* File mode.  */
    unsigned int st_nlink; /* Link count.  */
    unsigned int st_uid;   /* User ID of the file's owner.  */
    unsigned int st_gid;   /* Group ID of the file's group. */
    unsigned long st_rdev; /* Device number, if device.  */
    unsigned long __pad1;
    long st_size;   /* Size of file, in bytes.  */
    int st_blksize; /* Optimal block size for I/O.  */
    int __pad2;
    long st_blocks; /* Number 512-byte blocks allocated. */
    long st_atime;  /* Time of last access.  */
    unsigned long st_atime_nsec;
    long st_mtime; /* Time of last modification.  */
    unsigned long st_mtime_nsec;
    long st_ctime; /* Time of last status change.  */
    unsigned long st_ctime_nsec;
    unsigned int __unused4;
    unsigned int __unused5;
};

std::pair<uint32_t, bool> LinuxEmulator::handle_syscall(const Syscall &syscall)
{
    // https://github.com/riscv-collab/riscv-gnu-toolchain/blob/master/linux-headers/include/asm-generic/unistd.h
    switch (syscall.call_num)
    {
        case 57: // close
        {
            uint32_t fd = syscall.arg1;
            if (fd == 0 && fd == 1 && fd == 2)
            {
                return {0, false};
            }

            return {-1, false};
        }
        case 62: // llseek
        {
            return {-1, false};
        }
        case 63: // read
        {
            uint32_t fd = syscall.arg1;
            uint32_t buff_addr = syscall.arg2;
            uint32_t size = syscall.arg3;

            return {handle_read(fd, buff_addr, size), false};
        }
        case 64: // write
        {
            uint32_t fd = syscall.arg1;
            uint32_t buff_addr = syscall.arg2;
            uint32_t size = syscall.arg3;

            return {handle_write(fd, buff_addr, size), false};
        }
        case 80: // fstat
        {
            uint32_t fd = syscall.arg1;
            uint32_t stat_out = syscall.arg2;

            return {handle_fstat(fd, stat_out), false};
        }
        case 93: // exit
        {
            uint32_t exit_code = syscall.arg1;
            std::cout << "\nExit code = " << exit_code << '\n';
            return {0, true};
        }
        case 214: // brk
        {
            uint32_t addr = syscall.arg1;
            return {handle_brk(addr), false};
        }
        default:
            assert(false);
    }

    return {0, 0};
}

int32_t LinuxEmulator::handle_read(uint32_t fd, uint32_t buff_addr, uint32_t size)
{
    if (fd != 0)
    {
        return -1;
    }

    uint8_t *buf = new uint8_t[size];
    int32_t r = read(fd, buf, size);
    mmu.write_from(buff_addr, buf, buf + size);
    return r;

    /*
    for (uint32_t i = 0; i < size; ++i)
    {
        char c = getchar();
        if (c == '\n')
        {
            mmu.write(buff_addr + i, c);
            return i + 1;
        }

        mmu.write(buff_addr + i, c);
    }

    return size;
    */
}

int32_t LinuxEmulator::handle_write(uint32_t fd, uint32_t buff_addr, uint32_t size)
{
    if (fd != 1 && fd != 2)
    {
        return -1;
    }

    
    uint8_t *buf = new uint8_t[size];
    mmu.read_bunch(buff_addr, buf, size);
    int r = write(fd, buf, size);
    return r;

/*
    for (uint32_t i = 0; i < size; ++i)
    {
        char c = mmu.read<char>(buff_addr + i);
        std::cout << c << std::flush;
    }
*/
    return size;
}

int32_t LinuxEmulator::handle_fstat(uint32_t fd, uint32_t stat_out)
{
    struct stat st = {};

    st.st_dev = 26;
    st.st_ino = 6;
    st.st_mode = 8592;
    st.st_nlink = 1;
    st.st_uid = 1000;
    st.st_gid = 5;
    st.st_rdev = 0;
    st.st_size = 0;
    st.st_blksize = 1024;
    st.st_blocks = 0;
    st.st_atime = 2571619444006255626;
    st.st_atime_nsec = 0;
    st.st_mtime = 2571619444006226465;
    st.st_mtime_nsec = 0;
    st.st_ctime = 0;
    st.st_ctime_nsec = 0;
    mmu.write_from(stat_out, (uint8_t *)&st, (uint8_t *)&st + sizeof(st));

    return 0;
}

int32_t LinuxEmulator::handle_brk(uint32_t addr)
{
    if (addr == 0)
    {
        return mmu.get_brk_alloc();
    }
    
    const uint32_t alloc_size = addr - mmu.get_brk_alloc();
    mmu.allocate(alloc_size);
    return addr;
}
