//
// Created by M on 2023/12/16.
//

#include <stdint.h>
#ifndef CSAPP_COMMON_H
#define CSAPP_COMMON_H

#define DEBUG_INSTRUCTIONCYCLE      (0x1)
#define DEBUG_REGISTERS             (0x2)
#define DEBUG_PRINTSTACK            (0x4)
#define DEBUG_PRINTCACHESET         (0x8)
#define DEBUG_CACHEDETAILS          (0x10)
#define DEBUG_MMU                   (0x20)
#define DEBUG_LINKER                (0x40)
#define DEBUG_LOADER                (0x80)
#define DEBUG_PARSEINST             (0x100)

#define DEBUG_DATASTRUCTURE         (0x200)

#define DEBUG_VERBOSE_SET           (0x241)


#define DEBUG_ENABLE_PAGE_WALK      0

#define DEBUG_ENABLE_SRAM_CACHE     1

// printf wrapper
uint64_t debug_printf(uint64_t open_set, const char *format, ...);

// type converter
// uint32 to its equivalent float with rounding
uint32_t uint2float(uint32_t u);

// convert string dec or hex to the integer bitmap
uint64_t string2uint(const char *str);
uint64_t string2uint_range(const char *str, int start, int end);

// commonly shared variables
#define MAX_INSTRUCTION_CHAR 64


/*======================================*/
/*      clean up events                 */
/*======================================*/
void add_cleanup_event(void *func);
void finally_cleanup();
/*======================================*/
/*      wrap of the memory              */
/*======================================*/
void *tag_malloc(uint64_t size, char *tagstr);
int tag_free(void *ptr);
void tag_sweep(char *tagstr);

#endif //CSAPP_COMMON_H
