//
// Created by M on 2023/12/17.
//

// Memory Management Unit
#include<stdio.h>
#include <stdint.h>
#include<stdlib.h>
#include<assert.h>
#include <string.h>
#include "../../header/address.h"
#include "../../header/memory.h"
#include "../../header/common.h"
#include "../../header/cpu.h"




static uint64_t page_walk(uint64_t vaddr_value);
static void page_fault_handler(pte4_t *pte, address_t vaddr);

static int read_tlb(uint64_t vaddr_value, uint64_t *paddr_value_ptr,
                    int *free_tlb_line_index);
static int write_tlb(uint64_t vaddr_value, uint64_t paddr_value,
                     int free_tlb_line_index);

int swap_in(uint64_t daddr, uint64_t ppn);
int swap_out(uint64_t daddr, uint64_t ppn);

uint64_t va2pa(uint64_t vaddr, core_t *cr)
{
    uint64_t paddr = 0;
    #ifdef USE_TLB

    int free_tlb_line_index= -1;

    int hit_res = read_tlb(vaddr_value, &paddr, &free_tlb_line_index);

    if(hit_res == 1) {
        return paddr;
    }
    #endif
    #ifdef USE_HARDWARE
    paddr =  page_walk(vaddr);
    #endif
    #ifdef USE_TLB

    if(paddr != 0){

        if(write_tlb(vaddr_value, paddr, free_tlb_line_index) == 1){
            return paddr;
        }
    }

    #endif

    return paddr;
}


static int read_tlb(uint64_t vaddr_value, uint64_t *paddr_value_ptr,
                    int *free_tlb_line_index){

    address_t vaddr = {
            .vaddr_value = vaddr_value
    };
    *free_tlb_line_index = -1;

    tlb_cacheset_t *set = &mmu_tlb.sets[vaddr.tlbi];

    for (int i = 0; i < NUM_TLB_CACHE_LINE_PER_SET; i++) {

        tlb_cacheline_t *cacheline = &set->lines[i];

        if (cacheline->valid == 0) {
            *free_tlb_line_index = i;
        }
        if (cacheline->valid == 1 && cacheline->tag == vaddr.tlbt) {
            *paddr_value_ptr = cacheline->ppn;
            return 1;
        }
    }

    *paddr_value_ptr = NULL;
    return 0;
}

static int write_tlb(uint64_t vaddr_value, uint64_t paddr_value,
                     int free_tlb_line_index){

    address_t vaddr = {
            .address_value = vaddr_value
    };

    address_t paddr = {
            .address_value = paddr_value
    };

    if (0 < free_tlb_line_index && free_tlb_line_index < NUM_TLB_CACHE_LINE_PER_SET) {

        tlb_cacheset_t *set = &mmu_tlb.sets[vaddr.tlbi];
        tlb_cacheline_t *cacheline = &set->lines[free_tlb_line_index];

        cacheline->valid = 1;
        cacheline->tag = vaddr.tlbt;
        cacheline->ppn = paddr.ppn;

        return 1;
    }

    //not hit

    free_tlb_line_index = random() % NUM_TLB_CACHE_LINE_PER_SET;

    tlb_cacheset_t *set = &mmu_tlb.sets[vaddr.tlbi];
    tlb_cacheline_t *cacheline = &set->lines[free_tlb_line_index];


    cacheline->valid = 1;
    cacheline->tag = vaddr.tlbt;
    cacheline->ppn = paddr.ppn;

    return 1;
}

static uint64_t page_walk(uint64_t vaddr_value){

    address_t vaddr = {
            .vaddr_value = vaddr_value
    };

    //page table size 4kb
    uint64_t page_table_size = PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t);
    //level 1
    pte123_t *pgd = (pte123_t *) cpu_controls.cr3;
    assert(pgd!=NULL);


    if (pgd[vaddr.vpn1].present == 1) {
        //level 2
        pte123_t *pud = (pte123_t *) pgd[vaddr.vpn1].paddr;

        if (pud[vaddr.vpn2].present == 1) {
            //level 3
            pte123_t *pmd = (pte123_t *) pud[vaddr.vpn2].paddr;

            if (pmd[vaddr.vpn3].present == 1) {

                //level 4
                pte4_t *pt = (pte123_t *) pmd[vaddr.vpn3].paddr;

                if (pt[vaddr.vpn4].present == 1) {

                    address_t paddr = {
                            .ppn = pt[vaddr.vpn4].daddr,
                            .ppo = vaddr.ppo
                    };
                    return paddr.paddr_value;
                } else {
                    #ifdef DBUEG_PAGE_WALK
                    printf("page walk level 4: pt[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
                    #endif
                    page_fault_handler(&pt[vaddr.vpn4], vaddr);
                }

            } else {
                #ifdef DEBUG_PAGE_WALK
                    printf("page walk level 3: pmd[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn2);
                #endif
                page_fault_handler(&pmd[vaddr.vpn2], vaddr);

                pte123_t *pmd = malloc(page_table_size);
                memset(pmd, 0, page_table_size);
                pmd[vaddr.vpn2].present = 1;
                pmd[vaddr.vpn2].paddr = (uint64_t) pmd;
                exit(0);
            }

        } else {
            #ifdef DEBUG_PAGE_WALK
            printf("page walk level 2: pud[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn2);
            #endif
            page_fault_handler(&pud[vaddr.vpn2], vaddr);

            pte123_t *pud = malloc(page_table_size);
            memset(pud, 0, page_table_size);
            pud[vaddr.vpn2].present = 1;
            pud[vaddr.vpn2].paddr = (uint64_t) pud;
            exit(0);
        }

    } else {
        #ifdef DEBUG_PAGE_WALK
        printf("page walk level 1: pgd[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
        #endif
        page_fault_handler(&pgd[vaddr.vpn1], vaddr);


        pte123_t *pud = malloc(page_table_size);
        memset(pud, 0, page_table_size);
        pud[vaddr.vpn1].present = 1;
        pud[vaddr.vpn1].paddr = (uint64_t) pud;
        exit(0);
    }
}

static void page_fault_handler(pte4_t *pte, address_t vaddr){

    assert(pte->present == 0);

    int ppn = -1;
    pte4_t *victim = NULL;
    uint64_t daddr = 0xffffffffffffffff;

    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++i) {

        if (page_map[i].pte4->present == 0) {
            ppn = i;
            page_map[ppn].allocated = 1;
            page_map[ppn].time = 0;
            page_map[ppn].dirty = 0;
            page_map[ppn].pte4 = pte;

            pte->present = 1;
            pte->ppn = ppn;
            pte->dirty = 0;
            return;
        }
    }

    int lru_time = 0;
    int lru_ppn = -1;

    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++i) {

        if (page_map[i].dirty == 0 && page_map[i].time > lru_time) {

            lru_time = page_map[i].time;
            lru_ppn = i;
        }
    }

    if (lru_time != -1 && lru_ppn < MAX_NUM_PHYSICAL_PAGE) {

        ppn = lru_ppn;
        victim = page_map[ppn].pte4;
        victim->present = 0;
        victim->daddr = page_map[ppn].daddr;

        daddr = pte->daddr;
        swap_in(pte->daddr, ppn);

        pte->pte_value = 0;
        pte->present = 1;
        pte->ppn = ppn;
        pte->dirty = 0;

        page_map[ppn].daddr = daddr;
        page_map[ppn].pte4 = pte;
        page_map[ppn].time = 0;
        page_map[ppn].dirty = 0;
        page_map[ppn].allocated = 1;

        return;
    }

     lru_time = 0;
     lru_ppn = -1;

    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++i) {

        if (page_map[i].dirty == 1 && page_map[i].time > lru_time) {

            lru_time = page_map[i].time;
            lru_ppn = i;
        }
    }

    assert(0 <= lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE);

    ppn = lru_ppn;

    victim = page_map[ppn].pte4;
    swap_out(page_map[ppn].daddr, ppn);

    victim->pte_value = 0;
    victim->present = 1;
    victim-> ppn = ppn;
    victim->daddr = page_map[ppn].daddr;

    swap_in(pte->daddr, ppn);
    page_map[ppn].daddr = daddr;
    page_map[ppn].pte4 = pte;
    page_map[ppn].time = 0;
    page_map[ppn].dirty = 0;
    page_map[ppn].allocated = 1;
}
