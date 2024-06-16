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
                uint64_t PPO : PHYSICAL_PAGE_OFFSET_LENGTH;
                uint64_t PPN : PHYSICAL_PAGE_NUMBER_LENGTH;
            };
        };
    };

    // sram cache: 52
    struct
    {
        uint64_t CO : SRAM_CACHE_OFFSET_LENGTH;
        uint64_t CI : SRAM_CACHE_INDEX_LENGTH;
        uint64_t CT : SRAM_CACHE_TAG_LENGTH;
    };
} address_t;


#endif //CSAPP_ADDRESS_H
