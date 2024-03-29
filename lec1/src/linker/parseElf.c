//
// Created by M on 2024/1/29.
//

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<header/linker.h>
#include<header/common.h>
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


void parse_elf(char *filename,elf_t *elf){


    assert(elf!=NULL); 
    int line_count = read_elf(filename, (uint64_t) (&(elf->buffer)));
    int sh_count  = string2uint(elf->buffer[1]);
    elf->sht = malloc(sh_count*sizeof(sh_entry_t));
    for (int i = 0; i < line_count; i++)
    {
        parse_sh(elf->buffer[i+2],&(elf->sht[i]));
    }
    

}

static void parse_sh(char *str,sh_entry_t *sh){
    //声明一个指向指针数组的指针
    char ** cols;
    //
    int num = parse_table_entry(str,&cols)

}

//解析.text,0x0,4,22,放入到ent中
static int parse_table_entry(char *str, char *** ent){
    int col_count = 1;
    int len = strlen(str);
    //获取有效字符串的个数
    for (int i = 0; i < len; i++)
    {
        if(str[i]==','||str[i]=='\0'){
            col_count ++;
        }
    }
    int col_index = 0;
    int col_width = 0;
    char ** arr = malloc(col_count*sizeof(char *));
    * ent = arr;
    for (int i = 0; i < count; i++)
    {
        /* code */
    }
    


    
} 