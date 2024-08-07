//
// Created by M on 2024/6/5.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>
#include "../header/linker.h"
#include "../header/common.h"

const char *EXECUTABLE_DIRECTORY = "./files/exe";

// linker front end
int main(int argc, char **argv)
{
    char *elf_fn[64];
    char *eof_fn;
    int elf_num = 0;

    // parse the arguments
    int eof_flag = 0;
    for (int i = 1; i < argc; ++ i)
    {
        char *str = argv[i];
        if (strcmp(str, "-h") == 0 || strcmp(str, "--help") == 0)
        {
            printf("./bin/link <ELF file> ... <ELF file> -o <EOF file>\n");
            exit(0);
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            eof_flag = 1;
            continue;
        }

        if (eof_flag == 0)
        {
            // elf files
            elf_fn[elf_num] = str;
            elf_num ++;
        }
        else
        {
            // eof file
            eof_fn = str;
        }
    }

    // parsed the arguments

    // dynamically loading the shared library
    void *linklib = dlopen("./bin/staticLinker.so", RTLD_LAZY);
    if (linklib == NULL)
    {
        printf("%s\n", dlerror());
        exit(1);
    }

    // functions from shared library
    void (*link_elf)(elf_t **, int, elf_t *);
    void (*write_eof)(const char *, elf_t *);
    void (*parse_elf)(const char *, elf_t *);
    void (*free_elf)(elf_t *elf);

    link_elf = dlsym(linklib, "link_elf");
    write_eof = dlsym(linklib, "write_eof");
    parse_elf = dlsym(linklib, "parse_elf");
    free_elf = dlsym(linklib, "free_elf");

    // do front end logic

    printf("we are DYNAMICALLY LINKING ./bin/linker.so to do STATIC linking:\nlinking ");
    elf_t **srcs = tag_malloc(elf_num * sizeof(elf_t *), "link");
    for (int i = 0; i < elf_num; ++ i)
    {
        char elf_fullpath[100];
        sprintf(elf_fullpath, "%s/%s.elf.txt", EXECUTABLE_DIRECTORY, elf_fn[i]);
        printf("%s\n", elf_fullpath);

        srcs[i] = tag_malloc(sizeof(elf_t), "link");
        parse_elf(elf_fullpath, srcs[i]);
    }

    elf_t linked;
    link_elf(srcs, elf_num, &linked);

    char eof_fullpath[100];
    sprintf(eof_fullpath, "%s/%s.eof.txt", EXECUTABLE_DIRECTORY, eof_fn);
    printf("into %s\n", eof_fullpath);

    write_eof(eof_fullpath, &linked);

    // releaes elf heap
    for (int i = 0; i < elf_num; ++ i)
    {
        free_elf(srcs[i]);
    }

    tag_free(srcs);

    return 0;
}
