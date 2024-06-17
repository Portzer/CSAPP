//
// Created by M on 2024/6/16.
//
#include "../../header/address.h"
#include "../../header/memory.h"
#include <stdint.h>
#include <assert.h>
typedef enum
{
    CACHE_LINE_INVALID,
    CACHE_LINE_CLEAN,
    CACHE_LINE_DIRTY
} sram_cacheline_state_t;

typedef struct
{
    sram_cacheline_state_t state;
    int time;  // timer to find LRU line inside one set
    uint64_t tag;
    uint8_t block[(1 << SRAM_CACHE_OFFSET_LENGTH)];
} sram_cacheline_t;

typedef struct
{
    sram_cacheline_t lines[1 << NUM_CACHE_LINE_PER_SET];
} sram_cacheset_t;

typedef struct
{
    sram_cacheset_t sets[(1 << SRAM_CACHE_INDEX_LENGTH)];
} sram_cache_t;

static sram_cache_t cache;


uint8_t sram_cache_read(uint64_t address) {

    address_t paddr = {
            .paddr_value = address,
    };
    sram_cacheset_t cache_set = cache.sets[paddr.CI];
    sram_cacheline_t *invalid = NULL;
    sram_cacheline_t *victim = NULL;
    int max_time = -1;
    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; i++) {

        sram_cacheline_t *line = &cache_set.lines[i];
        line->time += 1;

        if (max_time < line->time) {
            max_time = line->time;
            victim = line;
        }
        if (line->state == CACHE_LINE_INVALID) {
            invalid = line;
        }

    }

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; i++) {

        sram_cacheline_t *line = &cache_set.lines[i];

        if (CACHE_LINE_INVALID != line->state && line->tag == paddr.CT) {
            line->time = 0;
            return line->block[paddr.CO];
        }

    }
    if (invalid != NULL) {
        read_bus_cacheline(paddr.address_value, &(invalid->block));
        invalid->state = CACHE_LINE_CLEAN;
        invalid->tag = paddr.CT;
        invalid->time = 0;
        return invalid->block[paddr.CO];
    }

    assert(victim!=NULL);
    if (victim->state == CACHE_LINE_DIRTY) {
        write_bus_cacheline(paddr.address_value, &(victim->block));
    }
    victim->state = CACHE_LINE_INVALID;
    read_bus_cacheline(paddr.address_value, &(victim->block));
    victim->state = CACHE_LINE_CLEAN;
    victim->time = 0;
    victim->tag = paddr.CT;
    return victim->block[paddr.CO];


}


void sram_cache_write(uint64_t address, uint8_t data) {
    address_t paddr = {
            .paddr_value = address,
    };
    sram_cacheset_t cache_set = cache.sets[paddr.CI];
    sram_cacheline_t *invalid = NULL;
    sram_cacheline_t *victim = NULL;
    int max_time = -1;
    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; i++) {

        sram_cacheline_t *line = &cache_set.lines[i];
        line->time += 1;

        if (max_time < line->time) {
            max_time = line->time;
            victim = line;
        }
        if (line->state == CACHE_LINE_INVALID) {
            invalid = line;
        }

    }

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; i++) {

        sram_cacheline_t *line = &cache_set.lines[i];

        if (CACHE_LINE_INVALID != line->state && line->tag == paddr.CT) {
            line->time = 0;
            line->block[paddr.CO] = data;
            line->state = CACHE_LINE_DIRTY;
            return;
        }

    }
    if (invalid != NULL) {
        read_bus_cacheline(paddr.address_value, &(invalid->block));
        invalid->state = CACHE_LINE_DIRTY;
        invalid->tag = paddr.CT;
        invalid->time = 0;
        invalid->block[paddr.CO] = data;
        return;
    }

    assert(victim != NULL);
    if (victim->state == CACHE_LINE_DIRTY) {
        write_bus_cacheline(paddr.address_value, &(victim->block));
    }
    victim->state = CACHE_LINE_INVALID;
    read_bus_cacheline(paddr.address_value, &(victim->block));
    victim->state = CACHE_LINE_CLEAN;
    victim->time = 0;
    victim->tag = paddr.CT;
    victim->block[paddr.CO] = data;

}




