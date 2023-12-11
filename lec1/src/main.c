#include <stdio.h>
#include <stdint.h>
#include "cpu/register.h"
#include "cpu/mmu.h"
#include "memory/instruction.h"
#include "disk/elf.h"
#include "memory/dram.h"

int main(){
    /**
     * x.c asm
start register
rax            0x1234              4660
rbx            0x555555555190      93824992235920
rcx            0x555555555190      93824992235920
rdx            0xabcd0000          2882338816
rsi            0x7fffffffe1f8      140737488347640
rdi            0x1                 1
rbp            0x7fffffffe100      0x7fffffffe100
rsp            0x7fffffffe0e0      0x7fffffffe0e0
r8             0x0                 0
r9             0x7ffff7fe0d60      140737354009952
r10            0x7ffff7ffcf68      140737354125160
r11            0x206               518
r12            0x555555555040      93824992235584
r13            0x7fffffffe1f0      140737488347632
r14            0x0                 0
r15            0x0                 0
rip            0x555555555173      0x555555555173 <main+37>
eflags         0x202               [ IF ]
cs             0x33                51
ss             0x2b                43
ds             0x0                 0
es             0x0                 0
fs             0x0                 0
gs             0x0                 0


end register
rax            0xabcd1234          2882343476
rbx            0x555555555190      93824992235920
rcx            0x555555555190      93824992235920
rdx            0x1234              4660
rsi            0xabcd0000          2882338816
rdi            0x1234              4660
rbp            0x7fffffffe100      0x7fffffffe100
rsp            0x7fffffffe0e0      0x7fffffffe0e0
r8             0x0                 0
r9             0x7ffff7fe0d60      140737354009952
r10            0x7ffff7ffcf68      140737354125160
r11            0x206               518
r12            0x555555555040      93824992235584
r13            0x7fffffffe1f0      140737488347632
r14            0x0                 0
r15            0x0                 0
rip            0x555555555182      0x555555555182 <main+52>
eflags         0x202               [ IF ]
cs             0x33                51
ss             0x2b                43
ds             0x0                 0
es             0x0                 0
fs             0x0                 0
gs             0x0                 0



start rsp (address) ~ (address + 10)
0x7fffffffe0e0: 0x00000000      0x00000000      0x00001234      0x00000000
0x7fffffffe0f0: 0xabcd0000      0x00000000      0x00000000      0x00000000
0x7fffffffe100: 0x00000000      0x00000000
end rsp (address) ~ (address + 10)
0x7fffffffe0e0: 0x00000000      0x00000000      0x00001234      0x00000000
0x7fffffffe0f0: 0xabcd0000      0x00000000      0xabcd1234      0x00000000
0x7fffffffe100: 0x00000000      0x00000000
**/

    init_handler_table();
    //init register info
    reg.rax = 0xabcd;
    reg.rbx = 0x555555555190;
    reg.rcx = 0x555555555190;
    reg.rdx = 0xabcd;
    reg.rsi = 0x7fffffffe1f8;
    reg.rdi = 0x1;
    reg.rbp = 0x7fffffffe100;
    reg.rsp = 0x7fffffffe0e0;
    reg.rip = (uint64_t)&program[11];

    //memory address info
    //rbp:函数的结束 rsp:函数的开始
    //        低地址:低地址/高地址 高地址:低地址/高地址
    //0x7fffffffe0f0: 0xabcd0000      0x00000000
//    mm[va2pa(0x7fffffffe100)] = 0x00000000;
//    mm[va2pa(0x7fffffffe0f8)] = 0x0;
//    mm[va2pa(0x7fffffffe0f0)] = 0xabcd;
//    mm[va2pa(0x7fffffffe0e8)] = 0x12340000;
//    mm[va2pa(0x7fffffffe0e0)] = 0x00000000 ;

    write64bits_dram(va2pa(0x7fffffffe100), 0x00000000);
    write64bits_dram(va2pa(0x7fffffffe0f8), 0x0);
    write64bits_dram(va2pa(0x7fffffffe0f0), 0xabcd);
    write64bits_dram(va2pa(0x7fffffffe0e8), 0x12340000);
    write64bits_dram(va2pa(0x7fffffffe0e0), 0x00000000);

    print_register();
    print_stack();

    for (int i = 0; i < 1; ++i){
        instruction_cycle();
        print_register();
        print_stack();
    }
    //验证
    int match = 1;
    match = match && (reg.rax == 0xabcd);
    match = match && (reg.rbx == 0x555555555190);
    match = match && (reg.rcx == 0x555555555190);
    match = match && (reg.rdx == 0x1234);
    match = match && (reg.rsi == 0xabcd0000);
    match = match && (reg.rdi == 0x1234);
    match = match && (reg.rbp == 0x7fffffffe100);
    match = match && (reg.rsp == 0x7fffffffe0e0);
    if (match == 1) {
        printf("reg match\n");
    } else {
        printf("reg no match\n");
    }
    read64bits_dram(va2pa(0x7fffffffe100));
    read64bits_dram(va2pa(0x7fffffffe0f8));
    read64bits_dram(va2pa(0x7fffffffe0f0));
    read64bits_dram(va2pa(0x7fffffffe0e8));
    read64bits_dram(va2pa(0x7fffffffe0e0));

    match = match && (read64bits_dram(va2pa(0x7fffffffe100)) == 0x00000000);
    match = match && (read64bits_dram(va2pa(0x7fffffffe0f8)) == 0x1234abcd);
    match = match && (read64bits_dram(va2pa(0x7fffffffe0f0)) ==  0xabcd);
    match = match && (read64bits_dram(va2pa(0x7fffffffe0e8)) ==  0x12340000);
    match = match && (read64bits_dram(va2pa(0x7fffffffe0e0)) == 0x00000000);
    if (match == 1) {
        printf("mem match\n");
    } else {
        printf("mem no match\n");
    }
    return 0;
}

