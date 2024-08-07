//
// Created by M on 2024/7/27.
//

#ifndef CSAPP_ALLOCATOR_H
#define CSAPP_ALLOCATOR_H

#include <stdint.h>

// heap's bytes range:
// [heap_start_vaddr, heap_end_vaddr) or [heap_start_vaddr, heap_end_vaddr - 1]
// [0,1,2,3] - unused
// [4,5,6,7,8,9,10,11] - prologue block
// [12, ..., 4096 * n - 5] - regular blocks
// 4096 * n + [- 4, -3, -2, -1] - epilogue block (header only)
uint64_t heap_start_vaddr;
uint64_t heap_end_vaddr;

#define HEAP_MAX_SIZE (4096 * 8)
uint8_t heap[HEAP_MAX_SIZE];

#define FREE (0)
#define ALLOCATED (1)
#define NIL (0)

// to allocate one physical page for heap
uint32_t extend_heap(uint32_t size);
void os_syscall_brk();

// round up
uint64_t round_up(uint64_t x, uint64_t n);

// operations for all blocks
uint32_t get_block_size(uint64_t header_vaddr);
void set_block_size(uint64_t header_vaddr, uint32_t blocksize);

uint32_t get_allocated(uint64_t header_vaddr);
void set_allocated(uint64_t header_vaddr, uint32_t allocated);

uint64_t get_payload(uint64_t vaddr);
uint64_t get_header(uint64_t vaddr);
uint64_t get_tail(uint64_t vaddr);

// operations for heap linked list

uint64_t get_next_header(uint64_t vaddr);
uint64_t get_prev_header(uint64_t vaddr);

uint64_t get_prologue();
uint64_t get_epilogue();

uint64_t get_first_block();
uint64_t get_last_block();

int is_last_block(uint64_t vaddr);
int is_first_block(uint64_t vaddr);

// for free block as data structure
uint64_t get_field32_block_ptr(uint64_t header_vaddr, uint32_t min_blocksize, uint32_t offset);
void set_field32_block_ptr(uint64_t header_vaddr, uint64_t block_ptr, uint32_t min_blocksize, uint32_t offset);

// common operations for malloc and free
uint64_t merge_blocks_as_free(uint64_t low, uint64_t high);
uint64_t try_alloc_with_splitting(uint64_t block_vaddr, uint32_t request_blocksize, uint32_t min_blocksize);
uint64_t try_extend_heap_to_alloc(uint32_t size, uint32_t min_blocksize);

// for debugging
void check_heap_correctness();
void print_heap();

#ifdef DEBUG_MALLOC
char debug_message[1000];
void on_sigabrt(int signum);
#endif

// interface
int heap_init();
uint64_t mem_alloc(uint32_t size);
void mem_free(uint64_t payload_vaddr);


#endif //CSAPP_ALLOCATOR_H
