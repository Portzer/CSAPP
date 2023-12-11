//
// Created by M on 2023/12/7.
//

#include "mmu.h"
#include "../memory/dram.h"


uint64_t va2pa(uint64_t va){
    return va % MM_LEN;
}
