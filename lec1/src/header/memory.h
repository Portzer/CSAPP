//
// Created by M on 2023/12/16.
//

#ifndef CSAPP_MEMORY_H
#define CSAPP_MEMORY_H

#include <stdint.h>
#include "cpu.h"

#define PHYSICAL_MEMORY_SPACE 65536
#define MAX_INDEX_PHYSICAL_PAGE 15
#define MAX_INSTRUCTION_CHAR 64

#define MAX_NUM_PHYSICAL_PAGE (16)
#define PAGE_TABLE_ENTRY_NUM    (512)

typedef union
{
    uint64_t pte_value;

    struct
    {
        uint64_t present            : 1;
        uint64_t readnly           : 1;
        uint64_t usermode           : 1;
        uint64_t writethough        : 1;
        uint64_t cachedisabled      : 1;
        uint64_t reference          : 1;
        uint64_t unused6            : 1;
        uint64_t smallpage          : 1;
        uint64_t global             : 1;
        uint64_t unused9_11         : 3;
        /*
        uint64_t paddr              : 40;
        uint64_t unused52_62        : 10;

        for malloc, a virtual address on heap is 48 bits
        for real world, a physical page number is 40 bits
        */
        uint64_t paddr              : 50;   // virtual address (48 bits) on simulator's heap
        uint64_t xdisabled          : 1;
    };

    struct
    {
        uint64_t _present           : 1;
        uint64_t daddr            : 63;   // disk address
    };
} pte123_t; // PGD, PUD, PMD


typedef union
{
    uint64_t pte_value;

    struct
    {
        uint64_t present            : 1;    // present = 1
        uint64_t readonly           : 1;
        uint64_t usermode           : 1;
        uint64_t writethough        : 1;
        uint64_t cachedisabled      : 1;
        uint64_t reference          : 1;
        uint64_t dirty              : 1;    // dirty bit - 1: dirty; 0: clean
        uint64_t zero7              : 1;
        uint64_t global             : 1;
        uint64_t unused9_11         : 3;
        uint64_t ppn                : 40;
        uint64_t unused52_62        : 10;
        uint64_t xdisabled          : 1;
    };

    struct
    {
        uint64_t _present           : 1;    // present = 0
        uint64_t daddr            : 63;   // disk address
    };
} pte4_t;   // PT

// physical page descriptor
typedef struct
{
    int allocated;
    int dirty;
    int time;   // LRU cache

    // real world: mapping to anon_vma or address_space
    // we simply the situation here
    // TODO: if multiple processes are using this page? E.g. Shared library
    pte4_t *pte4;       // the reversed mapping: from PPN to page table entry
    uint64_t daddr;   // binding the revesed mapping with mapping to disk
} pd_t;

pd_t page_map[MAX_NUM_PHYSICAL_PAGE];


uint8_t pm[PHYSICAL_MEMORY_SPACE];

uint64_t cpu_read64bits_dram(uint64_t paddr,core_t *core);

void cpu_write64bits_dram(uint64_t paddr, uint64_t data, core_t *core);


void cpu_writeinst_dram(uint64_t paddr, char *data,core_t *core);

void cpu_readinst_dram(uint64_t paddr, char  * buff ,core_t  *core);

void read_bus_cacheline(uint64_t paddr, uint8_t *block);

void write_bus_cacheline(uint64_t paddr,uint8_t * block);
#endif; //CSAPP_MEMORY_H
