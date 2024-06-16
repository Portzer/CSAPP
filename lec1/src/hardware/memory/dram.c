//
// Created by M on 2023/12/17.
//

// Dynamic Random Access Memory
#include"../../header/cpu.h"
#include"../../header/memory.h"
#include"../../header/common.h"
#include "../../header/address.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>
uint64_t read64bits_dram(uint64_t paddr, core_t *cr)
{
    if (DEBUG_ENABLE_SRAM_CACHE == 1)
    {
    }
    else
    {
        uint64_t val = 0x0;
        val += (((uint64_t)pm[paddr + 0 ]) << 0);
        val += (((uint64_t)pm[paddr + 1 ]) << 8);
        val += (((uint64_t)pm[paddr + 2 ]) << 16);
        val += (((uint64_t)pm[paddr + 3 ]) << 24);
        val += (((uint64_t)pm[paddr + 4 ]) << 32);
        val += (((uint64_t)pm[paddr + 5 ]) << 40);
        val += (((uint64_t)pm[paddr + 6 ]) << 48);
        val += (((uint64_t)pm[paddr + 7 ]) << 56);

        return val;
    }
}

void write64bits_dram(uint64_t paddr, uint64_t data, core_t *cr)
{
    if (DEBUG_ENABLE_SRAM_CACHE == 1)
    {
    }
    else
    {
        pm[paddr + 0] = (data >> 0 ) & 0xff;
        pm[paddr + 1] = (data >> 8 ) & 0xff;
        pm[paddr + 2] = (data >> 16) & 0xff;
        pm[paddr + 3] = (data >> 24) & 0xff;
        pm[paddr + 4] = (data >> 32) & 0xff;
        pm[paddr + 5] = (data >> 40) & 0xff;
        pm[paddr + 6] = (data >> 48) & 0xff;
        pm[paddr + 7] = (data >> 56) & 0xff;
    }
}

void writeinst_dram(uint64_t paddr, char *data, core_t *core)
{
        int len = strlen(data);
        assert(len < MAX_INSTRUCTION_CHAR);
    for (int i = 0; i < MAX_INSTRUCTION_CHAR; i++) {
        if (i < len) {
            pm[paddr + i] = data[i];
        } else {
            pm[paddr + i] = '\0';
        }
    }
}

void readinst_dram(uint64_t paddr, char *buf, core_t *core) {

    for (int i = 0; i < MAX_INSTRUCTION_CHAR; ++i) {
        buf[i] = pm[paddr + i];
    }
}


void read_bus_cacheline(uint64_t paddr, uint8_t *block) {

    uint64_t base_addr = ((paddr >> SRAM_CACHE_OFFSET_LENGTH) << SRAM_CACHE_OFFSET_LENGTH);

    for (int i = 0; i < 1 << SRAM_CACHE_OFFSET_LENGTH; ++i) {
        block[i] = pm[base_addr + i];
    }

}

void write_bus_cacheline(uint64_t paddr, uint8_t *block) {

    uint64_t base_addr = ((paddr >> SRAM_CACHE_OFFSET_LENGTH) << SRAM_CACHE_OFFSET_LENGTH);

    for (int i = 0; i < 1 << SRAM_CACHE_OFFSET_LENGTH; ++i) {
        pm[base_addr + i] = block[i];
    }
}
