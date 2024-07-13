//
// Created by M on 2024/6/16.
//

#ifndef CSAPP_ADDRESS_H
#define CSAPP_ADDRESS_H

#include <stdint.h>
#define NUM_CACHE_LINE_PER_SET (8)
#define SRAM_CACHE_TAG_LENGTH (40)
#define SRAM_CACHE_INDEX_LENGTH (6)
#define SRAM_CACHE_OFFSET_LENGTH (6)

#define PHYSICAL_PAGE_OFFSET_LENGTH (12)
#define PHYSICAL_PAGE_NUMBER_LENGTH (40)
#define PHYSICAL_ADDRESS_LENGTH (52)

#define VIRTUAL_PAGE_OFFSET_LENGTH (12)
#define VIRTUAL_PAGE_NUMBER_LENGTH (9)  // 9 + 9 + 9 + 9 = 36
#define VIRTUAL_ADDRESS_LENGTH (48)

#define TLB_CACHE_OFFSET_LENGTH (12)
#define TLB_CACHE_INDEX_LENGTH (4)
#define TLB_CACHE_TAG_LENGTH (32)
#define NUM_TLB_CACHE_LINE_PER_SET (8)



/*
+--------+--------+--------+--------+---------------+
|  VPN1  |  VPN2  |  VPN3  |  VPN4  |               |
+--------+--------+--------+-+------+      VPO      |
|    TLBT                    | TLBI |               |
+---------------+------------+------+---------------+
                |        PPN        |      PPO      |
                +-------------------+--------+------+
                |        CT         |   CI   |  CO  |
                +-------------------+--------+------+
*/
typedef union
{
    uint64_t address_value;

    // physical address: 52
    struct
    {
        union
        {
            uint64_t paddr_value : PHYSICAL_ADDRESS_LENGTH;
            struct
            {
                uint64_t ppo : PHYSICAL_PAGE_OFFSET_LENGTH;
                uint64_t ppn : PHYSICAL_PAGE_NUMBER_LENGTH;
            };
        };
    };

    // sram cache: 52
    struct
    {
        uint64_t co : SRAM_CACHE_OFFSET_LENGTH;
        uint64_t ci : SRAM_CACHE_INDEX_LENGTH;
        uint64_t ct : SRAM_CACHE_TAG_LENGTH;
    };

    // virtual address: 48
    struct
    {
        union
        {
            uint64_t vaddr_value : VIRTUAL_ADDRESS_LENGTH;
            struct
            {
                uint64_t vpo  : VIRTUAL_PAGE_OFFSET_LENGTH;
                uint64_t vpn4 : VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn3 : VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn2 : VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn1 : VIRTUAL_PAGE_NUMBER_LENGTH;
            };
        };
    };

    // TLB cache: 48
    struct
    {
        uint64_t tlbo : TLB_CACHE_OFFSET_LENGTH;   // virtual page offset
        uint64_t tlbi : TLB_CACHE_INDEX_LENGTH;    // TLB set index
        uint64_t tlbt : TLB_CACHE_TAG_LENGTH;      // TLB line tag
    };
} address_t;





typedef struct
{
    int valid;
    uint64_t tag;
    uint64_t ppn;
} tlb_cacheline_t;

typedef struct
{
    tlb_cacheline_t lines[NUM_TLB_CACHE_LINE_PER_SET];
} tlb_cacheset_t;

typedef struct
{
    tlb_cacheset_t sets[(1 << TLB_CACHE_INDEX_LENGTH)];
} tlb_cache_t;

static tlb_cache_t mmu_tlb;





#endif //CSAPP_ADDRESS_H
