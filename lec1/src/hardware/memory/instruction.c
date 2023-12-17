////
//// Created by M on 2023/12/7.
////
//
//#include "instruction.h"
//#include "../cpu/mmu.h"
//#include "../cpu/register.h"
//#include "dram.h"
//
//#include <stdio.h>
//
//
//static uint64_t decode_od(od_t od){
//    if (od.type == IMM) {
//        return *((uint64_t*)&od.imm);
//    } else if (od.type == REG) {
//        return (uint64_t)od.reg1;
//    } else{
//        uint64_t vaddr = 0;
//        if (od.type == MM_IMM) {
//            vaddr = od.imm;
//        } else if (od.type == MM_REG) {
//            vaddr = *(od.reg1);
//        } else if (od.type == MM_IMM_REG) {
//            vaddr = od.imm + *(od.reg1);
//        } else if (od.type == MM_REG1_REG2) {
//            vaddr =*(od.reg1) + *(od.reg2);
//        } else if (od.type == MM_IMM_REG1_REG2) {
//            vaddr = od.imm + *(od.reg1) + *(od.reg2);
//        }  else if (od.type == MM_REG2_S) {
//            vaddr = (*(od.reg2)) * od.scal;
//        } else if (od.type == MM_IMM_REG2_S) {
//            vaddr = od.imm +(*(od.reg2)) * od.scal;
//        } else if (od.type == MM_REG1_REG2_S) {
//            vaddr = *(od.reg1) + (*(od.reg2)) * od.scal;
//        }else if (od.type == MM_IMM_REG1_REG2_S) {
//            vaddr = od.imm + *(od.reg1) + (*(od.reg2)) * od.scal;
//        }
//        return vaddr;
//    }
//}
//void instruction_cycle(){
//    inst_t* inst = (inst_t *) reg.rip;
//    uint64_t src = decode_od(inst->src);
//    uint64_t dest = decode_od(inst->dst);
//    handle_t handler = handle_table[inst->op];
//    handler(src, dest);
//    printf("instruction is %s \n", inst->code);
//
//}
//
//void init_handler_table (){
//    handle_table[mov_reg_reg] = &mov_reg_reg_handler;
//    handle_table[mov_reg_mem] = &mov_reg_mem_handler;
//    handle_table[mov_mem_reg] = &mov_mem_reg_handler;
//    handle_table[push_reg] = &push_handler;
//    handle_table[pop_reg] = &pop_handler;
//    handle_table[call] = &call_handler;
//    handle_table[ret] = &ret_handler;
//    handle_table[add_reg_reg] = &add_reg_reg_handler;
//};
//
//
//void add_reg_reg_handler(uint64_t src, uint64_t dest){
//    *(uint64_t *) dest += *(uint64_t *) src;
//    reg.rip = reg.rip + sizeof(inst_t);
//
//}
//void mov_reg_reg_handler(uint64_t src, uint64_t dest){
//    *(uint64_t *) dest = *(uint64_t *) src;
//    reg.rip = reg.rip + sizeof(inst_t);
//
//}
//
//void mov_reg_mem_handler(uint64_t src, uint64_t dest){
//    write64bits_dram(va2pa(dest), *(uint64_t *) src);
//    reg.rip = reg.rip + sizeof(inst_t);
//}
//
//void mov_mem_reg_handler(uint64_t src, uint64_t dest){
//    *(uint64_t *)dest = read64bits_dram(va2pa(src));
//    reg.rip = reg.rip + sizeof(inst_t);
//}
//
//
//void call_handler(uint64_t src, uint64_t dest){
//    reg.rsp = reg.rsp - 8;
//    write64bits_dram(va2pa(reg.rsp), reg.rip + sizeof(inst_t));
//    reg.rip = src;
//
//}
//void pop_handler(uint64_t src, uint64_t dest){
//    *(uint64_t *) src = read64bits_dram(va2pa(reg.rsp));
//    reg.rsp = reg.rsp + 0x8;
//    reg.rip = reg.rip + sizeof(inst_t);
//}
//
//void push_handler(uint64_t src, uint64_t dest){
//    reg.rsp = reg.rsp - 0x8;
//    write64bits_dram(va2pa(reg.rsp), *(uint64_t *) src);
//    reg.rip = reg.rip + sizeof(inst_t);
//}
//
//void ret_handler(uint64_t src, uint64_t dest){
//    uint64_t ret_addr = read64bits_dram(va2pa(reg.rsp));
//    reg.rsp += 8;
//    reg.rip = ret_addr;
//}