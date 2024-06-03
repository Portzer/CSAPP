//
// Created by M on 2024/4/2.
//
#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include "../header/linker.h"
#include "../header/common.h"

#define MAX_STABLE_SIZE 64

typedef struct {
    elf_t *elf;
    st_entry_t *src;
    st_entry_t *dest;
} stable_t;



static void symbol_processing(elf_t **srcp, int count, elf_t *dest, stable_t *smap_table, int *smap_count);

static void simple_resulocation(elf_t *src_elf, st_entry_t *sym, stable_t *smap);

static int symbol_precedence(st_entry_t *sym);

static void compute_section_header(elf_t *dst, stable_t *smap_table, int *smap_count);

static void relocation_processing(elf_t **srcs, int num_srcs, elf_t *dst, stable_t *smap_table, int *smap_count);

static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst, stable_t *smap_table, int *smap_count);

static const char *get_stb_string(st_bind_t bind);
static const char *get_stt_string(st_type_t type);

static void R_X86_64_32_handler(elf_t *dst, sh_entry_t *sh,
                                int row_referencing, int col_referencing, int addend,
                                st_entry_t *sym_referenced);
static void R_X86_64_PC32_handler(elf_t *dst, sh_entry_t *sh,
                                  int row_referencing, int col_referencing, int addend,
                                  st_entry_t *sym_referenced);
static void R_X86_64_PLT32_handler(elf_t *dst, sh_entry_t *sh,
                                   int row_referencing, int col_referencing, int addend,
                                   st_entry_t *sym_referenced);

typedef void (*rela_handler_t)(elf_t *dst, sh_entry_t *sh,
                               int row_referencing, int col_referencing, int addend,
                               st_entry_t *sym_referenced);

static rela_handler_t handler_table[3] = {
        &R_X86_64_32_handler,       // 0
        &R_X86_64_PC32_handler,     // 1
        // linux commit b21ebf2: x86: Treat R_X86_64_PLT32 as R_X86_64_PC32
        &R_X86_64_PC32_handler,     // 2
};


void link_elf(elf_t **srcp, int count, elf_t *dest) {

    memset(dest, 0, sizeof(elf_t));
    int smap_count = 0;
    stable_t smap_table[MAX_STABLE_SIZE];
    symbol_processing(srcp, count, dest, (stable_t *) &smap_table, &smap_count);
    printf("----------------------\n");

    for (int i = 0; i < smap_count; ++ i)
    {
        st_entry_t *ste = smap_table[i].src;
        debug_printf(DEBUG_LINKER, "%s\t%d\t%d\t%s\t%d\t%d\n",
                     ste->st_name,
                     ste->bind,
                     ste->type,
                     ste->st_shndx,
                     ste->st_value,
                     ste->st_size);
    }
    compute_section_header(dest, smap_table, &smap_count);
    // malloc the dst.symt
    dest->symt_count = smap_count;
    dest->symt = malloc(dest->symt_count * sizeof(st_entry_t));
    merge_section(srcp, count, dest, smap_table, &smap_count);
    printf("----------------------\n");
    printf("after merging the sections\n");
    for (int i = 0; i < dest->line_count; ++ i)
    {
        printf("%s\n", dest->buffer[i]);
    }


    // relocating: update the relocation entries from ELF files into EOF buffer
    relocation_processing(srcp, count, dest, smap_table, &smap_count);

    // finally, check EOF file
    if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0)
    {
        printf("----\nfinal output EOF:\n");
        for (int i = 0; i < dest->line_count; ++ i)
        {
            printf("%s\n", dest->buffer[i]);
        }
    }


}


static void symbol_processing(elf_t **srcp, int count, elf_t *dest, stable_t *smap_table, int *smap_count) {
    for (int i = 0; i < count; ++i) {
        elf_t *src_elf = srcp[i];
        int sym_count = src_elf->symt_count;
        for (int j = 0; j < sym_count; ++j) {
            st_entry_t *sym = &(src_elf->symt[j]);
            if (sym->bind == STB_LOCAL) {
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = src_elf;
                (*smap_count)++;
                goto NEXT_SYMBOL_PROCESS;
            } else if (sym->bind == STB_GLOBAL) {
                for (int k = 0; k < *smap_count; ++k) {
                    st_entry_t *smap_src_sym = smap_table[k].src;
                    if (smap_src_sym->bind == STB_GLOBAL &&
                            strcmp(smap_src_sym->st_name, sym->st_name) == 0) {
                        simple_resulocation(src_elf, sym, &(smap_table[k]));
                        goto NEXT_SYMBOL_PROCESS;
                    }
                }
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = src_elf;
                (*smap_count)++;
            }
        }
        NEXT_SYMBOL_PROCESS:
        // do nothing
        ;
    }
    //整理smap_table 将common段换成bss段
    for (int i = 0; i < *smap_count; i++) {
        st_entry_t *s = smap_table[i].src;
        assert(strcmp(s->st_shndx, "SHN_UNDEF") != 0);
        assert(s->type != STT_NOTYPE);
        char *bss = ".bss";
        if (strcmp(s->st_shndx, "SHN_UNDEF") == 0) {
            if (i < 4) {
                s->st_shndx[i] = bss[i];
            } else {
                s->st_shndx[i] = '\0';
            }
        }
        s->st_value = 0;
    }
}

static void simple_resulocation(elf_t *src_elf, st_entry_t *sym, stable_t *smap) {
    //当前符号的优先级
    int pre1 = symbol_precedence(sym);
    //smap中符号的优先级
    int pre2 = symbol_precedence(smap->src);
    //如果都是2
    if (pre1 == 2 && pre2 == 2) {
        debug_printf(DEBUG_LINKER,
                     "symbol resolution: strong symbol \"%s\" is redeclared\n", sym->st_name);
        exit(0);
    } else if (pre1 != 2 && pre2 != 2) {
        if (pre1 > pre2) {
            smap->src = sym;
            smap->elf = src_elf;
        }
    } else if (pre1 == 2) {
        smap->src = sym;
        smap->elf = src_elf;
    }
}

static int symbol_precedence(st_entry_t *sym) {

    /*  we do not consider weak because it's very rare
        and we do not consider local because it's not conflicting

            bind        type        shndx               prec
            --------------------------------------------------
            global      notype      undef               0 - undefined
            global      object      common              1 - tentative
            global      object      data,bss,rodata     2 - defined
            global      func        text                2 - defined
    */
    assert(sym->bind == STB_GLOBAL);
    if (sym->type == STT_NOTYPE && strcmp(sym->st_shndx, "SHN_UNDEF") == 0) {
        return 0;
    } else if (sym->type == STT_OBJECT && strcmp(sym->st_shndx, "COMMON") == 0) {
        return 1;
    } else if ((sym->type == STT_OBJECT && strcmp(sym->st_shndx, ".data") == 0)||
            (sym->type == STT_OBJECT && strcmp(sym->st_shndx, ".bss") == 0)||
            (sym->type == STT_OBJECT && strcmp(sym->st_shndx, ".rodata") == 0)||
            (sym->type == STT_FUNC && strcmp(sym->st_shndx, ".text") == 0)
    ) {
        return 2;
    }
    debug_printf(DEBUG_LINKER, "symbol resolution: cannot determine the symbol \"%s\" precedence", sym->st_name);
    exit(0);
}


static void compute_section_header(elf_t *dst, stable_t *smap_table, int *smap_count){

    int text_count = 0, data_count = 0, rodata_count = 0;

    for (int i = 0; i < *smap_count; i++) {

        st_entry_t *st = smap_table[i].src;
        if (strcmp(st->st_shndx, ".text") == 0) {
            text_count += st->st_size;
        }
        else if (strcmp(st->st_shndx, ".rodata") == 0) {
            rodata_count += st->st_size;
        }
        else if (strcmp(st->st_shndx, ".data") == 0) {
            data_count += st->st_size;
        }
    }
    //获取总行数
    int section_header_count = (text_count > 0) + (rodata_count > 0) + (data_count > 0) + 1;
    //获取head的总行数
    int line_count = 2 + section_header_count + text_count + rodata_count + data_count + *smap_count;
    dst->sht_count = section_header_count;
    dst->line_count = line_count;
    sprintf(dst->buffer[0], "%ld", dst->line_count);
    sprintf(dst->buffer[1], "%ld", dst->sht_count);
    uint64_t text_runtime_addr = 0x00400000;
    uint64_t rodata_runtime_addr = text_runtime_addr + text_count * MAX_INSTRUCTION_CHAR * sizeof(char);
    uint64_t data_runtime_addr = rodata_runtime_addr + data_count + sizeof(uint64_t);
    uint64_t sym_runtime_addr = 0;

    int section_offset = 2 + section_header_count;

    dst->sht = malloc(section_header_count * sizeof(sh_entry_t));

    sh_entry_t *sh = NULL;
    int sh_index = 0;
    if (text_count > 0) {
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);
        strcpy(sh->sh_name, ".text");
        sh->sh_addr = text_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = text_count;
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);
        sh_index++;
        section_offset += sh->sh_size;
    }
    if (rodata_count > 0) {
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);
        strcpy(sh->sh_name, ".rodata");
        sh->sh_addr = rodata_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = rodata_count;
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);
        sh_index++;
        section_offset += sh->sh_size;
    }
    if (data_count > 0) {
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);
        strcpy(sh->sh_name, ".data");
        sh->sh_addr = data_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = data_count;
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);
        sh_index++;
        section_offset += sh->sh_size;
    }
    assert(sh_index < dst->sht_count);
    sh = &(dst->sht[sh_index]);

    // write the fields
    strcpy(sh->sh_name, ".symtab");
    sh->sh_addr = sym_runtime_addr;
    sh->sh_offset = section_offset;
    sh->sh_size = *smap_count;

    // write to buffer
    sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

    assert(sh_index + 1 == dst->sht_count);

    // print and check
    if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0)
    {
        printf("----------------------\n");
        printf("Destination ELF's SHT in Buffer:\n");
        for (int i = 0; i < 2 + dst->sht_count; ++ i)
        {
            printf("%s\n", dst->buffer[i]);
        }
    }

}


static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst, stable_t *smap_table, int *smap_count) {

    int line_written = 1 + 1 + dst->sht_count;
    int symt_written = 0;
    int sym_section_offset = 0;
    for (int sh_index = 0; sh_index < dst->sht_count; sh_index++) {
        sh_entry_t *target_sh = &dst->sht[sh_index];
        sym_section_offset = 0;
        //遍历所有的src
        for (int i = 0; i < num_srcs; i++) {
            //遍历所有header
            int src_section_header_index = -1;
            for (int j = 0; j < srcs[i]->sht_count; j++) {
                //如果匹配
                if (strcmp(target_sh->sh_name, srcs[i]->sht[j].sh_name) == 0) {
                    //保留当前src文件中命中header的索引
                    src_section_header_index = j;
                }
            }
            //如果没有命中，则继续循环
            if (src_section_header_index == -1) {
                continue;
            }
            //命中则遍历src的symbol
            for (int j = 0; j < srcs[i]->symt_count; j++) {

                st_entry_t *st = &srcs[i]->symt[j];
                //如果有符号和当前段的类型一样
                if (strcmp(st->st_shndx, target_sh->sh_name) == 0) {
                    //遍历smap，匹配符号
                    for (int k = 0; k < *smap_count; ++k) {
                        if (st == smap_table[k].src) {
                            debug_printf(DEBUG_LINKER, "\t\tsymbol '%s'\n", st->st_name);
                            //匹配中符号，开始讲src复制到dest
                            for (int t = 0; t < st->st_size; t++) {
                                int dest_index = line_written + t;
                                int src_index = srcs[i]->sht[src_section_header_index].sh_offset + st->st_value + t;
                                assert(dest_index < MAX_ELF_FILE_LENGTH);
                                assert(src_index < MAX_ELF_FILE_LENGTH);
                                strcpy(dst->buffer[dest_index], srcs[i]->buffer[src_index]);
                            }
                            assert(symt_written < dst->symt_count);
                            strcpy(dst->symt[symt_written].st_name, st->st_name);
                            dst->symt[symt_written].bind = st->bind;
                            dst->symt[symt_written].type = st->type;
                            strcpy(dst->symt[symt_written].st_shndx, st->st_shndx);
                            // MUST NOT BE A COMMON, so the section offset MUST NOT BE alignment
                            dst->symt[symt_written].st_value = sym_section_offset;
                            dst->symt[symt_written].st_size = st->st_size;
                            smap_table[k].dest = &dst->symt[symt_written];
                            symt_written += 1;
                            printf("symbol '%ld' copied\n", st->st_size);
                            line_written += st->st_size;
                            sym_section_offset += st->st_size;
                            printf("sym is %s sym_section_offset is %d \n", st->st_name, sym_section_offset);
                        }
                    }
                }
            }
        }
    }
    for (int i = 0; i < dst->symt_count; ++ i)
    {
        st_entry_t *sym = &dst->symt[i];
        sprintf(dst->buffer[line_written], "%s,%s,%s,%s,%ld,%ld",
                sym->st_name, get_stb_string(sym->bind), get_stt_string(sym->type),
                sym->st_shndx, sym->st_value, sym->st_size);
        line_written ++;
    }
    assert(line_written == dst->line_count);

}



static void relocation_processing(elf_t **srcs, int num_srcs, elf_t *dst, stable_t *smap_table, int *smap_count){

    sh_entry_t *eof_text_sh = NULL;
    sh_entry_t *eof_data_sh = NULL;
    for (int i = 0; i < dst->sht_count; ++i) {
        if (strcmp(dst->sht[i].sh_name, ".text") == 0) {
            eof_text_sh = &(dst->sht[i]);
        }else if (strcmp(dst->sht[i].sh_name, ".data") == 0) {
            eof_data_sh = &(dst->sht[i]);
        }
    }
    for (int i = 0; i < num_srcs; ++i) {

        elf_t *src = srcs[i];
        for (int j = 0; j < src->reldata_count; ++j) {
            rl_entry_t *rl = &src->reldata[j];

            for (int k = 0; k < src->symt_count; ++k) {
                st_entry_t *sym = &src->symt[k];

                if (strcmp(src->symt[k].st_shndx, ".data") == 0) {
                    int sym_start = sym->st_value;
                    int sym_end = sym->st_value + sym->st_size;
                    if (rl->r_row >= sym_start && rl->r_row <= sym_end) {
                        int smap_found = 0;
                        for (int l = 0; l < *smap_count; ++l) {
                            if (smap_table[l].src == &src->symt[k]) {
                                smap_found = 1;
                                st_entry_t *st_referencing = smap_table[l].dest;
                                for (int m = 0; m < *smap_count; ++m) {
                                    if (strcmp(src->symt[rl->sym].st_name, smap_table[m].dest->st_name) == 0 &&
                                        smap_table[m].dest->bind == STB_GLOBAL)
                                    {
                                        st_entry_t *eof_referenced = smap_table[m].dest;
                                        (handler_table[(int) rl->type])(
                                                dst, eof_data_sh,
                                                rl->r_row - sym->st_value + st_referencing->st_value - 1,
                                                rl->r_col,
                                                rl->r_addend,
                                                eof_referenced);
                                        goto NEXT_REFERENCE_IN_DATA;
                                    }
                                }
                            }
                        }
                        assert(smap_found==1);
                    }
                    NEXT_REFERENCE_IN_DATA:
                    ;
                }
            }
        }
        for (int j = 0; j < src->reltext_count; ++j) {

            rl_entry_t *rl = &src->reltext[j];

            for (int k = 0; k < src->symt_count; ++k) {

                st_entry_t *sym = &src->symt[k];
                if (strcmp(sym->st_shndx, ".text") == 0) {

                    int sym_text_start = sym->st_value;
                    int sym_text_end =  sym->st_value + sym->st_size;
                    if (rl->r_row >= sym_text_start && rl->r_row <= sym_text_end) {

                        int smap_found = 0;
                        for (int l = 0; l < *smap_count; ++l) {
                            if (smap_table[l].src == sym) {
                                smap_found = 1;
                                st_entry_t *st_referencing = smap_table[l].dest;
                                for (int m = 0; m < *smap_count; ++m) {

                                    if (strcmp(src->symt[rl->sym].st_name, smap_table[m].dest->st_name) == 0 &&
                                        smap_table[m].dest->bind == STB_GLOBAL) {

                                        st_entry_t *eof_referenced = smap_table[m].dest;

                                        (handler_table[(int) rl->type])(
                                                dst, eof_text_sh,
                                                rl->r_row - sym->st_value + st_referencing->st_value - 1,
                                                rl->r_col,
                                                rl->r_addend,
                                                eof_referenced);
                                        goto NEXT_REFERENCE_IN_TEXT;
                                    }
                                }
                            }
                        }
                        assert(smap_found==1);
                    }
                }
            }
            NEXT_REFERENCE_IN_TEXT:
            ;
        }
    }
}


// relocating handlers

static void R_X86_64_32_handler(elf_t *dst, sh_entry_t *sh,
                                int row_referencing, int col_referencing, int addend,
                                st_entry_t *sym_referenced)
{
    printf("row = %d, col = %d, symbol referenced = %s\n",
           row_referencing, col_referencing, sym_referenced->st_name
    );
}

static void R_X86_64_PC32_handler(elf_t *dst, sh_entry_t *sh,
                                  int row_referencing, int col_referencing, int addend,
                                  st_entry_t *sym_referenced)
{
    printf("row = %d, col = %d, symbol referenced = %s\n",
           row_referencing, col_referencing, sym_referenced->st_name
    );
}

static void R_X86_64_PLT32_handler(elf_t *dst, sh_entry_t *sh,
                                   int row_referencing, int col_referencing, int addend,
                                   st_entry_t *sym_referenced)
{
    printf("row = %d, col = %d, symbol referenced = %s\n",
           row_referencing, col_referencing, sym_referenced->st_name
    );
}

static const char *get_stb_string(st_bind_t bind)
{
    switch (bind)
    {
        case STB_GLOBAL:
            return "STB_GLOBAL";
        case STB_LOCAL:
            return "STB_LOCAL";
        case STB_WEAK:
            return "STB_WEAK";
        default:
            printf("incorrect symbol bind\n");
            exit(0);
    }
}

static const char *get_stt_string(st_type_t type)
{
    switch (type)
    {
        case STT_NOTYPE:
            return "STT_NOTYPE";
        case STT_OBJECT:
            return "STT_OBJECT";
        case STT_FUNC:
            return "STT_FUNC";
        default:
            printf("incorrect symbol type\n");
            exit(0);
    }
}