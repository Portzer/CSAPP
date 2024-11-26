//
// Created by M on 2024/11/26.
//
#include <stdint.h>
#ifndef PROCESS_H
#define PROCESS_H
#define     KERNEL_STACK_SIZE   (8192)

typedef union KERNEL_STACK_STRUCT
{
    uint8_t stack[KERNEL_STACK_SIZE];
    // TODO: add thread_info
} kstack_t;

void syscall_init();
void do_syscall(int syscall_no);

#endif //PROCESS_H
