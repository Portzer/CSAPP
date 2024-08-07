//
// Created by M on 2024/1/29.
//

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include "../header/linker.h"
#include "../header/common.h"
int read_elf(const char *filename, uint64_t bufaddr)
{
    // open file and read
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        debug_printf(DEBUG_LINKER, "unable to open file %s\n", filename);
        exit(1);
    }

    // read text file line by line
    char line[MAX_ELF_FILE_WIDTH];
    int line_counter = 0;

    while (fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL)
    {
        int len = strlen(line);
        if ((len == 0) ||
            (len >= 1 && (line[0] == '\n' || line[0] == '\r')) ||
            (len >= 2 && (line[0] == '/' && line[1] == '/')))
        {
            continue;
        }

        // check if is empty or white line
        int iswhite = 1;
        for (int i = 0; i < len; ++ i)
        {
            iswhite = iswhite && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r');
        }
        if (iswhite == 1)
        {
            continue;
        }

        // to this line, this line is not white and contains information

        if (line_counter < MAX_ELF_FILE_LENGTH)
        {
            // store this line to buffer[line_counter]
            uint64_t addr = bufaddr + line_counter * MAX_ELF_FILE_WIDTH * sizeof(char);
            char *linebuf = (char *)addr;

            int i = 0;
            while (i < len && i < MAX_ELF_FILE_WIDTH)
            {
                if ((line[i] == '\n') ||
                    (line[i] == '\r') ||
                    ((i + 1 < len) && (i + 1 < MAX_ELF_FILE_WIDTH) && line[i] == '/' && line[i + 1] == '/'))
                {
                    break;
                }
                linebuf[i] = line[i];
                i ++;
            }
            linebuf[i] = '\0';
            line_counter ++;
        }
        else
        {
            debug_printf(DEBUG_LINKER, "elf file %s is too long (>%d)\n", filename, MAX_ELF_FILE_LENGTH);
            fclose(fp);
            exit(1);
        }
    }

    fclose(fp);
    assert(string2uint((char *)bufaddr) == line_counter);
    return line_counter;
}


static void init_dictionary()
{
    if (link_constant_dict != NULL)
    {
        return;
    }

    link_constant_dict = hashtable_construct(4);
    hashtable_insert(&link_constant_dict, "STB_LOCAL", STB_LOCAL);
    hashtable_insert(&link_constant_dict, "STB_GLOBAL", STB_GLOBAL);
    hashtable_insert(&link_constant_dict, "STB_WEAK", STB_WEAK);

    hashtable_insert(&link_constant_dict, "STT_NOTYPE", STT_NOTYPE);
    hashtable_insert(&link_constant_dict, "STT_OBJECT", STT_OBJECT);
    hashtable_insert(&link_constant_dict, "STT_FUNC", STT_FUNC);

    hashtable_insert(&link_constant_dict, "R_X86_64_32", R_X86_64_32);
    hashtable_insert(&link_constant_dict, "R_X86_64_PC32", R_X86_64_PC32);
    hashtable_insert(&link_constant_dict, "R_X86_64_PLT32", R_X86_64_PLT32);

    print_hashtable(link_constant_dict);
}




//解析.text,0x0,4,22,放入到ent中
static int parse_table_entry(char *str, char *** ent){
    int col_count = 1;
    int len = strlen(str);
    //获取有效字符串的个数
    for (int i = 0; i < len; i++)
    {
        if(str[i]==','){
            col_count ++;
        }
    }
    int col_index = 0;
    int col_width = 0;
    char col_buffer[32];
    char **arr = malloc(col_count * sizeof(char *));
    * ent = arr;
    for (int j = 0; j < len + 1; j++) {
        if (str[j] == ',' || str[j] == '\0') {
            assert(col_index < col_count);
            char *col = malloc((col_width + 1) * sizeof(char));
            for (int k = 0; k < col_width; k++) {
                col[k] = col_buffer[k];
            }
            col[col_width] = '\0';
            arr[col_index] = col;
            col_index++;
            col_width = 0;
        } else {
            assert(col_width<32);
            col_buffer[col_width] = str[j];
            col_width++;
        }
    }
    return col_count;
}


static void free_table_entry(char **ent, int n)
{
    for (int i = 0; i < n; ++ i)
    {
        free(ent[i]);
    }
    free(ent);
}

static void print_sh_entry(sh_entry_t *sh)
{
    debug_printf(DEBUG_LINKER, "%s\t%x\t%d\t%d\n",
        sh->sh_name,
        sh->sh_addr,
        sh->sh_offset,
        sh->sh_size);
}



static void print_symtab_entry(st_entry_t *ste)
{
    debug_printf(DEBUG_LINKER, "%s\t%d\t%d\t%s\t%d\t%d\n",
                 ste->st_name,
                 ste->bind,
                 ste->type,
                 ste->st_shndx,
                 ste->st_value,
                 ste->st_size);
}

static void print_relocation_entry(rl_entry_t *rte)
{
    debug_printf(DEBUG_LINKER, "relocation entry: %d\t%d\t%d\t%d\t%d\n",
                 rte->r_row,
                 rte->r_col,
                 rte->type,
                 rte->sym,
                 rte->r_addend);
}

static void parse_symtab(char *str,st_entry_t *st){
    //sum,STB_GLOBAL,STT_FUNCTION,.text,0,22
    char **cols;
    int num_col = parse_table_entry(str, &cols);
    assert(num_col==6);
    assert(st!=NULL);
    strcpy(st->st_name,cols[0]);

    // select symbol bind
    uint64_t bind_value;
    if (hashtable_get(link_constant_dict, cols[1], &bind_value) == 0)
    {
        // failed
        printf("symbol bind is neiter LOCAL, GLOBAL, nor WEAK\n");
        exit(0);
    }
    st->bind = (st_bind_t)bind_value;

    uint64_t type_value;
    if (hashtable_get(link_constant_dict, cols[2], &type_value) == 0)
    {
        // failed
        printf("symbol type is neiter NOTYPE, OBJECT, nor FUNC\n");
        exit(0);
    }
    st->type = (st_type_t)type_value;

    strcpy(st->st_shndx, cols[3]);

    st->st_value = string2uint(cols[4]);
    st->st_size = string2uint(cols[5]);

    free_table_entry(cols, num_col);
}
static void parse_sh(char *str,sh_entry_t *sh){
    //声明一个指向指针数组的指针
    char ** cols;
    //
    int num_col = parse_table_entry(str,&cols);
    assert(num_col==4);
    assert(sh!=NULL);
    strcpy(sh->sh_name,cols[0]);
    sh->sh_addr = string2uint(cols[1]);
    sh->sh_offset = string2uint(cols[2]);
    sh->sh_size = string2uint(cols[3]);
    free_table_entry(cols,num_col);
}

static void parse_relocation(char *str, rl_entry_t *rel) {
    /// 4,7,R_X86_64_PC32,0,-4
    char **cols;
    int num_col = parse_table_entry(str, &cols);
    assert(num_col==5);
    assert(rel!=NULL);
    rel->r_row = string2uint(cols[0]);
    rel->r_col = string2uint(cols[1]);
    uint64_t type_value;
    if (hashtable_get(link_constant_dict, cols[2], &type_value) == 0)
    {
        printf("relocation type is neiter R_X86_64_32, R_X86_64_PC32, nor R_X86_64_PLT32\n");
        exit(0);
    }
    rel->type = (st_type_t)type_value;

    rel->sym = string2uint(cols[3]);
    uint64_t  bitmap = string2uint(cols[4]);
    rel->r_addend = *(int64_t *) &bitmap;
    free_table_entry(cols, num_col);
}

void parse_elf(char *filename,elf_t *elf){


    assert(elf!=NULL);
    int line_count = read_elf(filename, (uint64_t) (&(elf->buffer)));
    for (int i = 0; i < line_count; ++ i)
    {
        printf("[%d]\t%s\n", i, elf->buffer[i]);
    }
    elf->sht_count = string2uint(elf->buffer[1]);;
    elf->sht = malloc(elf->sht_count *sizeof(sh_entry_t));
    sh_entry_t *smyt_sh = NULL;
    sh_entry_t *rel_text_sh = NULL;
    sh_entry_t *rel_data_sh= NULL;

    init_dictionary();

    for (int i = 0; i < elf->sht_count; i++)
    {
        parse_sh(elf->buffer[i + 2], &(elf->sht[i]));
        print_sh_entry(&(elf->sht[i]));
        //获取符号表.symtab,0x0,26,2
        if (strcmp(elf->sht[i].sh_name, ".symtab") == 0) {
            smyt_sh = &(elf->sht[i]);
        }else if (strcmp(elf->sht[i].sh_name, ".rel.text") == 0){
            rel_text_sh = &(elf->sht[i]);
        } else if (strcmp(elf->sht[i].sh_name, ".rel.data") == 0) {
            rel_data_sh = &(elf->sht[i]);
        }
    }

    assert(smyt_sh!=NULL);
    elf->symt_count = smyt_sh->sh_size;
    elf->symt = malloc(elf->symt_count * sizeof(st_entry_t));
    for (int i = 0; i < smyt_sh->sh_size; i++) {
        parse_symtab(elf->buffer[i + smyt_sh->sh_offset], &(elf->symt[i]));
        print_symtab_entry(&(elf->symt[i]));
    }
    if (rel_text_sh != NULL) {
        elf->reltext_count = rel_text_sh->sh_size;
        elf->reltext = malloc(elf->reltext_count * sizeof(rl_entry_t));
        for (int i = 0; i < rel_text_sh->sh_size; i++) {
            parse_relocation(
                    elf->buffer[i + rel_text_sh->sh_offset],
                    &(elf->reltext[i])
            );
            int st = elf->reltext[i].sym;
            assert(0 <= st && st < elf->symt_count);

            print_relocation_entry(&(elf->reltext[i]));
        }
    }else{
        elf->reltext_count = 0;
        elf->reltext = NULL;
    }
    if (rel_data_sh != NULL) {
        elf->reldata_count = rel_data_sh->sh_size;
        elf->reldata = malloc(elf->reldata_count * sizeof(rl_entry_t));
        for (int i = 0; i < rel_data_sh->sh_size; i++) {
            parse_relocation(
                    elf->buffer[i + rel_data_sh->sh_offset],
                    &(elf->reldata[i])
            );
            int st = elf->reldata[i].sym;
            assert(0 <= st && st < elf->symt_count);

            print_relocation_entry(&(elf->reldata[i]));
        }
    }else{
        elf->reldata_count = 0;
        elf->reldata = NULL;
    }

}


void write_eof(const char *filename, elf_t *eof)
{
    // open elf file
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        debug_printf(DEBUG_LINKER, "unable to open file %s\n", filename);
        exit(1);
    }

    for (int i = 0; i < eof->line_count; ++ i)
    {
        fprintf(fp, "%s\n", eof->buffer[i]);
    }

    fclose(fp);

    // free hash table
    hashtable_free(link_constant_dict);
}

void free_elf(elf_t *elf)
{
    assert(elf != NULL);

    free(elf->sht);
}


