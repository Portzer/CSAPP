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
uint8_t pm[PHYSICAL_MEMORY_SPACE];

uint64_t read64bits_dram(uint64_t paddr,core_t *core);

void write64bits_dram(uint64_t paddr, uint64_t data, core_t *core);


void writeinst_dram(uint64_t paddr, char *data,core_t *core);

void readinst_dram(uint64_t paddr, char  * buff ,core_t  *core);

void read_bus_cacheline(uint64_t paddr, uint8_t *block);

void write_bus_cacheline(uint64_t paddr,uint8_t * block);
#endif; //CSAPP_MEMORY_H
