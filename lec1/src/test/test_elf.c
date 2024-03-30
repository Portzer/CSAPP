//
// Created by M on 2024/1/29.
//
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>

int read_elf(const char *filename, uint64_t bufaddr);
int parse_table_entry(char *str, char ***ent);
#include "../header/linker.h"
#include "../header/common.h"

void parse_sh(char *str, sh_entry_t *sh);
void free_table_entry(char **ent, int n);
void print_sh_entry(sh_entry_t *sh);
void parse_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);

int main()
{
    elf_t elf;
    parse_elf("./files/exe/sum_elf.text", &elf);
    free_elf(&elf);
}