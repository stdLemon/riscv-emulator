#pragma once

#include <cinttypes>

struct Syscall
{
    uint32_t call_num;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
    uint32_t arg5;
    uint32_t arg6;
};