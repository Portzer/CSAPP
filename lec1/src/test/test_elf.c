//
// Created by M on 2024/1/29.
//
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include "../header/linker.h"
#include "../header/common.h"


int main()
{
    elf_t src[2];
    elf_t dest;
    parse_elf("./files/exe/main_elf.text",&src[0]);
    parse_elf("./files/exe/sum_elf.text", &src[1]);
    elf_t * srcp[2];
    srcp[0] = &src[0];
    srcp[1] = &src[1];

    link_elf((elf_t **) &srcp, 2, &dest);
    write_eof("./files/exe/output.eof.txt", &dest);
    free_elf(&src[0]);
    free_elf(&src[1]);
    free_elf(&dest);
}