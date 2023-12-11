//
// Created by M on 2023/12/7.
//

#include <stdlib.h>
#include "../memory/instruction.h"
#include "../cpu/register.h"
//模拟decode
inst_t program[15]={
    {push_reg,
     {REG,0,0,(uint64_t*)&reg.rbp,NULL},
     {EMPTY,0,0,NULL,NULL},
     "push   \%rbp"
     },

    {mov_reg_reg,
     {REG,0,0,(uint64_t *)&reg.rsp,NULL},
     {REG,0,0,(uint64_t *)&reg.rbp,NULL},
     "mov    \%rsp,\%rbp"
     },

    {mov_reg_mem,
     {REG,0,0,(uint64_t *)&reg.rdi,NULL},
     {MM_IMM_REG,-0x18,0,(uint64_t *)&reg.rbp,NULL},
     "mov    \%rdi,-0x18(\%rbp)"
     },

    {mov_reg_mem,
     {REG,0,0,(uint64_t *)&reg.rsi,NULL},
     {MM_IMM_REG,-0x20,0,(uint64_t *)&reg.rbp,NULL},
     " \%rsi,-0x20(\%rbp)"
     },

    {mov_reg_mem,
     {MM_IMM_REG,-0x18,0,(uint64_t *)&reg.rbp,NULL},
     {REG,0,0,(uint64_t *)&reg.rdx,NULL},
     "mov    -0x18(\%rbp),\%rdx"
     },

    {mov_reg_mem,
     {MM_IMM_REG,-0x20,0,(uint64_t *)&reg.rbp,NULL},
     {REG,0,0,(uint64_t *)&reg.rax,NULL},
     "mov    -0x20(\%rbp),\%rax"
    },

    {add_reg_reg,
     {REG,0,0,(uint64_t *)&reg.rdx,NULL},
     {REG,0,0,(uint64_t *)&reg.rax,NULL},
     "add    \%rdx,\%rax"
     },

   {mov_reg_mem,
    {REG,0,0,(uint64_t *)&reg.rax,NULL},
    {MM_IMM_REG,-0x8,0,(uint64_t *)&reg.rbp,NULL},
    "mov    \%rax,-0x8(\%rbp)"
    },

    {mov_reg_mem,
     {MM_IMM_REG,-0x8,0,(uint64_t *)&reg.rbp,NULL},
     {REG,0,0,(uint64_t *)&reg.rax,NULL},
     "mov    -0x8(\%rbp),\%rax"
     },

    {pop_reg,
     {REG,0,0,(uint64_t*)&reg.rbp,NULL},
     {EMPTY,0,0,NULL,NULL},
     "pop    \%rbp"
     },

    {ret,
    {EMPTY,0,0,NULL,NULL},
    {EMPTY,0,0,NULL,NULL},
    "retq"
     },


 ////////////////////////////////////////////////////////////////

    {mov_reg_reg,
     {REG,0,0,(uint64_t *)&reg.rdx,NULL},
     {REG,0,0,(uint64_t *)&reg.rsi,NULL},
     "mov    \%rdx,\%rsi"
     },

    {mov_reg_reg,
     {REG,0,0,(uint64_t *)&reg.rax,NULL},
     {REG,0,0,(uint64_t *)&reg.rdi,NULL},
     "mov    \%rax,\%rdi"
     },

    {call,
     //add的地址就是objdump.asm的起始地址
     {IMM,(uint64_t)&program[0],0,NULL,NULL},
     {EMPTY,0,0,NULL,NULL},
     "call add"
    },

    {mov_reg_mem,
     {REG,0,0,(uint64_t *)&reg.rax,NULL},
     {MM_IMM_REG,-0x8,0,(uint64_t *)&reg.rbp,NULL},
     "mov    \%rax,-0x8(\%rbp)"
    }

};
