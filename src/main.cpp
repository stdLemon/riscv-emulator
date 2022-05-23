#include "elf-loader/elf-loader.hpp"
#include "mmu/mmu.hpp"
#include "riscv-emulator/riscv-emulator.hpp"

/*
0001008c <_start>:
   1008c:       00005197                auipc   x3,0x5
   10090:       de418193                addi    x3,x3,-540 # 14e70 <__global_pointer$>
   10094:       04418513                addi    x10,x3,68 # 14eb4 <__malloc_max_total_mem>
   10098:       09c18613                addi    x12,x3,156 # 14f0c <__BSS_END__>
   1009c:       40a60633                sub     x12,x12,x10
   100a0:       00000593                addi    x11,x0,0
   100a4:       198000ef                jal     x1,1023c <memset>
   100a8:       00002517                auipc   x10,0x2
   100ac:       29050513                addi    x10,x10,656 # 12338 <atexit>
   100b0:       00050863                beq     x10,x0,100c0 <_start+0x34>
   100b4:       00001517                auipc   x10,0x1
   100b8:       88450513                addi    x10,x10,-1916 # 10938 <__libc_fini_array>
   100bc:       27c020ef                jal     x1,12338 <atexit>
   100c0:       0e0000ef                jal     x1,101a0 <__libc_init_array>
   100c4:       00012503                lw      x10,0(x2)
   100c8:       00410593                addi    x11,x2,4
   100cc:       00000613                addi    x12,x0,0
   100d0:       06c000ef                jal     x1,1013c <main>
   100d4:       09c0006f                jal     x0,10170 <exit>
*/
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 1;
    }

    const char *executable_path = argv[1];

    Mmu mmu(1024 * 1024 * 100);
    ElfLoader elf_loader(mmu);

    uint32_t entry_point = elf_loader.load(executable_path);
    RiscvEmulator emulator(mmu);
    emulator.run(entry_point);
    return 0;
}
