//
// Created by M on 2023/12/17.
//

// Memory Management Unit
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<header/cpu.h>
#include<header/memory.h>
#include<header/common.h>

uint64_t va2pa(uint64_t vaddr, core_t *cr)
{
    return vaddr & (0xffffffffffffffff >> (64 - MAX_INDEX_PHYSICAL_PAGE));
}