//
// Created by M on 2024/7/12.
//

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/common.h"
#include "../../header/address.h"

// each swap file is swap page
// each line of this swap page is one uint64
#define SWAP_PAGE_FILE_LINES 512

// disk address counter
static uint64_t internal_swap_daddr = 0;

int swap_in(uint64_t daddr,uint64_t ppn) {


    char filename[128];
    sprintf(filename, "../files/swap/page-%ld.txt", daddr);\

    FILE *fr = fopen(filename, "r");
    assert(fr!=NULL);
    char buf[64] = {'0'};

    for (int i = 0; i < SWAP_PAGE_FILE_LINES; ++i) {

        char *str = fgets(buf, 64, fr);

        uint64_t ppo = ppn << PHYSICAL_PAGE_OFFSET_LENGTH;
        assert(str != NULL);
        *((uint64_t *) (&pm[ppo + i * 8])) = string2uint(str);
    }
    fclose(fr);
}


int swap_out(uint64_t paddr, uint64_t ppn) {

    FILE *fw = NULL;
    char filename[128];
    sprintf(filename, "../files/swap/page-%ld.txt", daddr);
    fw = fopen(filename, "w");
    assert(fw != NULL);

    uint64_t ppn_ppo = ppn << PHYSICAL_PAGE_OFFSET_LENGTH;
    for (int i = 0; i < SWAP_PAGE_FILE_LINES; ++ i)
    {
        fprintf(fw, "0x%16lx\n", *((uint64_t *)(&pm[ppn_ppo + i * 8])));
    }
    fclose(fw);
}