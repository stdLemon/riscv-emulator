#include "riscv-emulator.hpp"
#include "../elf-loader/elf-loader.hpp"

#include "instruction-formats/bType.hpp"
#include "instruction-formats/iType.hpp"
#include "instruction-formats/jType.hpp"
#include "instruction-formats/rType.hpp"
#include "instruction-formats/sType.hpp"
#include "instruction-formats/uType.hpp"
#include <cstdint>
#include <iostream>

void RiscvEmulator::run(uint32_t entry_point)
{
    set_pc(entry_point);

    const uint32_t stack_size = 1024 * 1024 * 2; // 2MB
    uint32_t stack_addr = mmu.get_first_alloc() - 1;
    
    if(stack_addr < stack_size)
    {
        stack_addr = mmu.allocate(stack_size) + stack_size - 1;
    }

    set_register(RegisterName::sp, stack_addr);
    while (true)
    {
        const uint32_t inst = fetch_instruction();
   
        execute_instruction(inst);

        if (!running)
        {
            break;
        }
        if (!skip_pc_update)
        {
            next_pc();
        }
        else
        {
            skip_pc_update = false;
        }
    }
}

uint32_t RiscvEmulator::fetch_instruction() const
{
    const uint32_t pc = get_pc();
    const uint32_t inst = mmu.read<uint32_t>(pc);

    if(pc == 0x10160){
        std::cout << "break";
    }
    std::cerr << "fetch from 0x" << std::hex << pc << " inst = 0x" << inst << '\n';
    return inst;
}

void RiscvEmulator::execute_instruction(uint32_t inst)
{
    uint8_t opcode = inst & 0b1111111;
    std::cerr << "decoded opcode = " << std::bitset<7>(opcode) << '\n';

    switch (opcode)
    {
        case 0b0110111:
        {
            /*
                LUI (load upper immediate) is used to build 32-bit constants and uses the U-type format. LUI
                places the U-immediate value in the top 20 bits of the destination register rd, filling in the lowest
                12 bits with zeros
            */

            const Utype u_type = Utype::from(inst);
            const uint32_t value = u_type.imm << 12;

            std::cerr << "LUI " << u_type << '\n';
            set_register(u_type.rd, value);
            break;
        }

        case 0b0010111:
        {
            /*
                AUIPC (add upper immediate to pc) is used to build pc-relative addresses and uses the U-type
                format. AUIPC forms a 32-bit offset from the 20-bit U-immediate, filling in the lowest 12 bits with
                zeros, adds this offset to the address of the AUIPC instruction, then places the result in register rd.
            */

            const Utype u_type = Utype::from(inst);
            const uint32_t offset = u_type.imm << 12;

            std::cerr << "AUIPC " << u_type << '\n';
            set_register(u_type.rd, get_pc() + offset);
            break;
        }
        case 0b1101111:
        {
            /*
                The jump and link (JAL) instruction uses the J-type format, where the J-immediate encodes a
                signed offset in multiples of 2 bytes. The offset is sign-extended and added to the address of the
                jump instruction to form the jump target address. Jumps can therefore target a ±1 MiB range.
                JAL stores the address of the instruction following the jump (pc+4) into register rd. The standard
                software calling convention uses x1 as the return address register and x5 as an alternate link register.
                Register x0 can be used as the destination if the result is not required.
            */

            const Jtype j_type = Jtype::from(inst);
            const uint32_t target = get_pc() + j_type.imm;
            const uint32_t ret = get_pc() + sizeof(uint32_t);

            if (j_type.rd != (uint8_t)RegisterName::zero)
            {
                set_register(j_type.rd, ret);
            }
            set_pc(target);
            skip_pc_update = true;
            std::cerr << "JAL " << j_type << '\n';
            break;
        }
        case 0b1100111:
        {
            /*
                The indirect jump instruction JALR (jump and link register) uses the I-type encoding. The target
                address is obtained by adding the sign-extended 12-bit I-immediate to the register rs1, then setting
                the least-significant bit of the result to zero. The address of the instruction following the jump
                (pc+4) is written to register rd. 
                Register x0 can be used as the destination if the result is not required.
            */

            const Itype i_type = Itype::from(inst);
            const uint32_t target = i_type.imm + get_register(i_type.rs1);
            const uint32_t ret = get_pc() + sizeof(uint32_t);

            if (i_type.rd != (uint8_t)RegisterName::zero)
            {
                set_register(i_type.rd, ret);
            }
            set_pc(target);
            skip_pc_update = true;
            std::cerr << "JALR " << i_type << '\n';
            break;
        }
        case 0b1100011:
        {
            /*
                All branch instructions use the B-type instruction format. The 12-bit B-immediate encodes signed
                offsets in multiples of 2 bytes. The offset is sign-extended and added to the address of the branch
                instruction to give the target address. The conditional branch range is ±4 KiB.
            */

            const Btype b_type = Btype::from(inst);
            const uint32_t target = b_type.imm + get_pc();
            const uint32_t rs1 = get_register(b_type.rs1);
            const uint32_t rs2 = get_register(b_type.rs2);
            bool should_take_branch = false;

            switch (b_type.func3)
            {
                case 0b000:
                {
                    // BEQ take the branch if registers rs1 and rs2 are equal

                    should_take_branch = rs1 == rs2;
                    std::cerr << "BEQ ";
                    break;
                }
                case 0b001:
                {
                    // BNE take the branch if registers rs1 and rs2 are unequal

                    should_take_branch = rs1 != rs2;
                    std::cerr << "BNE ";
                    break;
                }
                case 0b100:
                {
                    // BLT take the branch if rs1 is less than rs2, using signed comparison

                    should_take_branch = (int32_t)rs1 < (int32_t)rs2;
                    std::cerr << "BLT ";
                    break;
                }
                case 0b101:
                {
                    // BGE take the branch if rs1 is greater than or equal to rs2, using signed comparison

                    should_take_branch = (int32_t)rs1 >= (int32_t)rs2;
                    std::cerr << "BGE ";
                    break;
                }
                case 0b110:
                {
                    // BLTU take the branch if rs1 is less than rs2, using unsigned comparison

                    should_take_branch = rs1 < rs2;
                    std::cerr << "BLTU ";
                    break;
                }
                case 0b111:
                {
                    // BGEU take the branch if rs1 is greater than or equal to rs2, using unsigned comparison

                    should_take_branch = rs1 >= rs2;
                    std::cerr << "BGEU ";
                    break;
                }
            }

            if (should_take_branch)
            {
                set_pc(target);
                skip_pc_update = true;
            }
            std::cerr << b_type << '\n';
            break;
        }
        case 0b0000011:
        {
            /*
                The effective byte address is obtained by adding register rs1
                to the sign-extended 12-bit offset.

                The LW instruction loads a 32-bit value from memory into rd. 
                LH loads a 16-bit value from memory, then sign-extends to 32-bits before storing in rd. 
                LHU loads a 16-bit value from memory but then zero extends to 32-bits before storing in rd. 
                LB and LBU are defined analogously for 8-bit values.
            */

            const Itype i_type = Itype::from(inst);
            const uint32_t load_address = get_register(i_type.rs1) + i_type.imm;

            std::cerr << i_type << '\n';
            switch (i_type.func3)
            {
                case 0b000:
                {
                    set_register(i_type.rd, (int32_t)mmu.read<uint8_t>(load_address));
                    std::cerr << "LB ";
                    break;
                }
                case 0b001:
                {
                    set_register(i_type.rd, (int32_t)mmu.read<uint16_t>(load_address));
                    std::cerr << "LH ";
                    break;
                }
                case 0b010:
                {
                    set_register(i_type.rd, mmu.read<uint32_t>(load_address));
                    std::cerr << "LW ";
                    break;
                }
                case 0b100:
                {
                    set_register(i_type.rd, mmu.read<uint8_t>(load_address));
                    std::cerr << "LBU ";
                    break;
                }
                case 0b101:
                {
                    std::cerr << "LHU ";
                    set_register(i_type.rd, mmu.read<uint16_t>(load_address));
                    break;
                }
            }
            break;
        }
        case 0b0100011:
        {
            /*
                The effective byte address is obtained by adding register rs1
                to the sign-extended 12-bit offset. 

                The SW, SH, and SB instructions store 
                32-bit, 16-bit, and 8-bit values from the low bits of register rs2 to memory.
            */

            const Stype s_type = Stype::from(inst);
            const uint32_t store_address = get_register(s_type.rs1) + s_type.imm;
            const uint32_t rs2 = get_register(s_type.rs2);

            switch (s_type.func3)
            {
                case 0b000:
                {
                    std::cerr << "SB ";
                    mmu.write<uint8_t>(store_address, rs2);
                    break;
                }
                case 0b001:
                {
                    std::cerr << "SH ";
                    mmu.write<uint16_t>(store_address, rs2);
                    break;
                }
                case 0b010:
                {
                    std::cerr << "SW ";
                    mmu.write<uint32_t>(store_address, rs2);
                    break;
                }
            }
            std::cerr << s_type << '\n';
            break;
        }
        case 0b0010011:
        {
            const Itype i_type = Itype::from(inst);
            const uint32_t rs1 = get_register(i_type.rs1);

            switch (i_type.func3)
            {
                case 0b000:
                {
                    /*
                        ADDI adds the sign-extended 12-bit immediate to register rs1. Arithmetic overflow is ignored and
                        the result is simply the low XLEN bits of the result. ADDI rd, rs1, 0 is used to implement the MV
                        rd, rs1 assembler pseudoinstruction
                    */
                    if (i_type.rd == 0 && i_type.imm == 0 && rs1 == 0)
                    {
                        /*
                            The NOP instruction does not change any architecturally visible state, except for advancing the
                            pc and incrementing any applicable performance counters. NOP is encoded as ADDI x0, x0, 0
                        */
                        std::cerr << "NOP ";
                        break;
                    }
                    set_register(i_type.rd, i_type.imm + rs1);
                    std::cerr << "ADDI ";
                    break;
                }
                case 0b010:
                {
                    /*
                        SLTI (set less than immediate) places the value 1 in register rd if register rs1 is less than the signextended immediate when both are treated as signed numbers, 
                        else 0 is written to rd.
                    */

                    set_register(i_type.rd, rs1 < i_type.imm);
                    std::cerr << "SLTI ";
                    break;
                }
                case 0b011:
                {
                    /*
                        SLTIU is similar but compares the values as unsigned numbers (i.e., the immediate is first sign-extended to
                        XLEN bits then treated as an unsigned number). Note, SLTIU rd, rs1, 1 sets rd to 1 if rs1 equals
                        zero, otherwise sets rd to 0 (assembler pseudoinstruction SEQZ rd, rs).
                    */

                    set_register(i_type.rd, rs1 < (uint32_t)i_type.imm);
                    std::cerr << "SLTIU ";
                    break;
                }
                case 0b100:
                {
                    /*
                        ANDI, ORI, XORI are logical operations that perform bitwise AND, OR, and XOR on register rs1
                        and the sign-extended 12-bit immediate and place the result in rd. Note, XORI rd, rs1, -1 performs
                        a bitwise logical inversion of register rs1 (assembler pseudoinstruction NOT rd, rs).
                    */

                    std::cerr << "XORI ";
                    set_register(i_type.rd, rs1 ^ i_type.imm);
                    break;
                }
                case 0b110:
                {
                    std::cerr << "ORI ";
                    set_register(i_type.rd, rs1 | i_type.imm);
                    break;
                }
                case 0b111:
                {
                    std::cerr << "ANDI ";
                    set_register(i_type.rd, rs1 & i_type.imm);
                    break;
                }
                case 0b001:
                {
                    const uint8_t shamt = i_type.imm & 0b11111;
                    std::cerr << "SLLI ";
                    set_register(i_type.rd, rs1 << shamt);
                    break;
                }
                case 0b101:
                {
                    /*
                        Shifts by a constant are encoded as a specialization of the I-type format. 
                        The operand to be shifted is in rs1, and the shift amount is encoded in the lower 5 bits of the I-immediate field. 
                        The right shift type is encoded in bit 30. 
                        SLLI is a logical left shift (zeros are shifted into the lower bits);
                        SRLI is a logical right shift (zeros are shifted into the upper bits); 
                        and SRAI is an arithmetic right shift (the original sign bit is copied into the vacated upper bits).
                    */

                    const uint8_t shamt = i_type.imm & 0b11111;
                    const uint8_t mode = i_type.imm >> 5;
                    const uint32_t rs1 = get_register(i_type.rs1);

                    switch (mode)
                    {
                        case 0b0000000:
                        {
                            std::cerr << "SRLI";
                            set_register(i_type.rd, rs1 >> shamt);
                            break;
                        }
                        case 0b0100000:
                        {
                            std::cerr << "SRAI";
                            set_register(i_type.rd, (int32_t)rs1 >> shamt);
                            break;
                        }
                    }
                    break;
                }
            }

            std::cerr << i_type << '\n';
            break;
        }

        case 0b0110011:
        {
            const Rtype r_type = Rtype::from(inst);
            const uint32_t rs1 = get_register(r_type.rs1);
            const int32_t rs2 = get_register(r_type.rs2);

            switch (r_type.func3)
            {
                case 0b000:
                {
                    /*
                        ADD performs the addition of rs1 and rs2. SUB performs the subtraction of rs2 from rs1. Overflows
                        are ignored and the low XLEN bits of results are written to the destination rd.
                    */

                    switch (r_type.func7)
                    {
                        case 0b0000000:
                        {
                            std::cerr << "ADD ";
                            set_register(r_type.rd, rs1 + rs2);
                            break;
                        }
                        case 0b0100000:
                        {
                            std::cerr << "SUB ";
                            set_register(r_type.rd, rs1 - rs2);
                            break;
                        }
                        /*
                        case 0b0000001:
                        {
                            switch(r_type.func3)
                            {
                                case 0b000:
                                {
                                    set_register(r_type.rd, rs1 * rs2);
                                    break;
                                }
                            }
                            break;
                        }
                        */
                        default:
                        {
                            assert(false);
                        }
                    }

                    break;
                }
                case 0b001:
                {
                    /*
                        SLL performs logical left shift
                    */

                    const uint8_t shamt = rs2 & 0b11111;
                    set_register(r_type.rd, rs1 << shamt);

                    std::cerr << "SLL ";
                    break;
                }
                case 0b010:
                {
                    /*
                        SLT perform signed compare writing 1 to rd if rs1 < rs2, 0 otherwise
                        rd, x0, rs2 sets rd to 1 if rs2 is not equal to zero, otherwise sets rd to zero 
                        (assembler pseudoinstruction SNEZ rd, rs).
                    */

                    set_register(r_type.rd, (int32_t)rs1 < (int32_t)rs2);
                    std::cerr << "SLT ";
                    break;
                }
                case 0b011:
                {
                    /*
                        SLTU perform unsigned compare writing 1 to rd if rs1 < rs2, 0 otherwise
                        SLTU rd, x0, rs2 sets rd to 1 if rs2 is not equal to zero, otherwise sets rd to zero 
                        (assembler pseudoinstruction SNEZ rd, rs).
                    */

                    set_register(r_type.rd, rs1 < rs2);
                    std::cerr << "SLTU ";
                    break;
                }
                case 0b100:
                {
                    std::cerr << "XOR ";
                    set_register(r_type.rd, rs1 ^ rs2);
                    break;
                }
                case 0b101:
                {
                    const uint8_t shamt = rs2 & 0b11111;

                    switch (r_type.func7)
                    {
                        case 0b0000000:
                        {
                            std::cerr << "SRL ";
                            set_register(r_type.rd, rs1 >> shamt);
                            break;
                        }
                        case 0b0100000:
                        {
                            std::cerr << "SRA ";
                            set_register(r_type.rd, (int32_t)rs1 >> shamt);
                            break;
                        }
                    }
                    break;
                }
                case 0b110:
                {
                    std::cerr << "OR ";
                    set_register(r_type.rd, rs1 | rs2);
                    break;
                }
                case 0b111:
                {
                    std::cerr << "AND ";
                    set_register(r_type.rd, rs1 & rs2);
                    break;
                }
            }

            std::cerr << r_type << '\n';
            break;
        }

        case 0b0001111:
        {
            std::cerr << "FENCE\n";
            throw std::invalid_argument("Unsupported FENCE");
            break;
        }
        case 0b1110011:
        {
            const Itype i_type = Itype::from(inst);
            const uint32_t funct12 = i_type.imm;

            switch (funct12)
            {
                case 0b000000000000:
                {
                    Syscall syscall{
                        .call_num = get_register(RegisterName::a7),
                        .arg1 = get_register(RegisterName::a0),
                        .arg2 = get_register(RegisterName::a1),
                        .arg3 = get_register(RegisterName::a2),
                        .arg4 = get_register(RegisterName::a3),
                        .arg5 = get_register(RegisterName::a4)};

                    std::cerr << "ECALL " << std::dec << syscall.call_num << std::endl;
                    auto [ret, exit] = linux_emulator.handle_syscall(syscall);
                    if (exit)
                    {
                        running = false;
                        break;
                    }

                    set_register(RegisterName::a0, ret);
                    break;
                }
                case 000000000001:
                {
                    std::cout << "EBREAK\n";
                    throw std::invalid_argument("Unsupported EBREAK");
                    break;
                }
            }
            break;
        }
        default:
        {
            throw std::invalid_argument("Unknown opcode");
        }
    }
}
