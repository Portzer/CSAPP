//
// Created by M on 2024/11/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "../header/cpu.h"
#include "../header/memory.h"
#include "../header/interrupt.h"

typedef void (*syscall_handler)();

typedef struct SYSCALL_ENTRY_STRUCT
{
    syscall_handler handler;
} syscall_entry_t;

// table of syscalls
syscall_entry_t syscall_table[64];

// handlers of syscalls
static void write_handler();
static void getpid_handler();
static void fork_handler();
static void execve_handler();
static void exit_handler();
static void wait_handler();
static void kill_handler();


void syscall_init()
{
    syscall_table[01].handler = write_handler;
    syscall_table[39].handler = getpid_handler;
    syscall_table[57].handler = fork_handler;
    syscall_table[59].handler = execve_handler;
    syscall_table[60].handler = exit_handler;
    syscall_table[61].handler = wait_handler;
    syscall_table[62].handler = kill_handler;
}

static void write_handler()
{
    uint64_t fd = cpu_reg.rdi;
    uint64_t buf_vaddr = cpu_reg.rsi;
    uint64_t buf_length = cpu_reg.rdx;

    for (uint64_t i = 0; i < buf_length; i++) {
        printf("%c", pm[va2pa(buf_vaddr + i)])
    }
}

static void getpid_handler()
{}

static void fork_handler()
{}

static void execve_handler()
{}



