//
// Created by M on 2024/11/26.
//
#include <stdint.h>
#ifndef INTERRUPT_H
#define INTERRUPT_H


// we only use stack0 of TSS
// This information is stored in main memory
typedef struct TSS_S0
{
    uint64_t ESP0;
    uint64_t SS0;
} tss_s0_t;

void idt_init();

void call_interrupt_stack_switching(uint64_t int_vec);


#endif //INTERRUPT_H
