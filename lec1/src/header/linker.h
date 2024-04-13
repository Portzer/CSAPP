//
// Created by M on 2024/1/11.
//

#ifndef CSAPP_LINKER_H
#define CSAPP_LINKER_H
#include <stdlib.h>
#include <stdint.h>
#define MAX_CHAR_SECTION_NAME 32
#define MAX_CHAR_SYMBOL_NAME (64)

//section header
typedef struct {
    char sh_name[MAX_CHAR_SECTION_NAME];
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
} sh_entry_t;

typedef enum{
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK
}st_bind_t;

typedef enum {
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC
} st_type_t;

//sym
typedef struct
{
    char st_name[MAX_CHAR_SECTION_NAME];
    st_bind_t bind;
    st_type_t type;
    char st_shndx[MAX_CHAR_SECTION_NAME];
    uint64_t st_value;      // in-section offset
    uint64_t st_size;       // count of lines of symbol
} st_entry_t;

#define MAX_ELF_FILE_LENGTH (64)    // max 64 effective lines
#define MAX_ELF_FILE_WIDTH (128)    // max 128 chars per line

typedef struct
{
    char buffer[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];
    uint64_t line_count;

    uint64_t sht_count;
    sh_entry_t *sht;

    uint64_t symt_count;
    st_entry_t *symt;
} elf_t;

void parse_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);

void link_elf(elf_t **src, int count, elf_t *dest);
#endif //CSAPP_LINKER_H

