//
// Created by M on 2023/12/7.
//

#ifndef inst_guard
#define inst_guard

#include <stdint.h>
#include <stdio.h>
#define MEM_LEN 1024
#define NUM_INSTR_TYPE 30



typedef enum OP{
    mov_reg_reg,
    mov_reg_mem,
    mov_mem_reg,
    push_reg,
    pop_reg,
    call,
    ret,
    add_reg_reg
} op_t;

typedef enum OD_TYPE{
    IMM,REG,
    MM_IMM,MM_REG,MM_IMM_REG,MM_REG1_REG2,
    MM_IMM_REG1_REG2, MM_REG2_S, MM_IMM_REG2_S,
    MM_REG1_REG2_S, MM_IMM_REG1_REG2_S,EMPTY
}od_type_t;

typedef struct OD{
    od_type_t type;
    int64_t imm;
    uint64_t scal;
    uint64_t *reg1;
    uint64_t *reg2;
} od_t;

typedef struct INSTRUCT_STRUCT{
    op_t op;
    od_t src;
    od_t dst;
    char code[100];
}inst_t;


void instruction_cycle();


typedef void (*handle_t)(uint64_t, uint64_t);

handle_t handle_table[NUM_INSTR_TYPE];


void init_handler_table();

void add_reg_reg_handler(uint64_t stc, uint64_t dest);

void mov_reg_reg_handler(uint64_t src, uint64_t dest);

void call_handler(uint64_t src, uint64_t dest);

void pop_handler(uint64_t src, uint64_t dest);

void push_handler(uint64_t src, uint64_t dest);

void mov_reg_mem_handler(uint64_t src, uint64_t dest);
#endif




