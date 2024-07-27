//
// Created by M on 2024/7/27.
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include "../header/allocator.h"

static int internal_heap_init();
static uint64_t internal_malloc(uint32_t size);
static void internal_free(uint64_t payload_vaddr);

/* ------------------------------------- */
/*  Implementation of the Interfaces     */
/* ------------------------------------- */

#ifdef REDBLACK_TREE

int heap_init()
{
    return internal_heap_init();
}

uint64_t mem_alloc(uint32_t size)
{
    return internal_malloc(size);
}

void mem_free(uint64_t payload_vaddr)
{
    internal_free(payload_vaddr);
}

#ifdef DEBUG_MALLOC
void on_sigabrt(int signum)
{
    // like a try-catch for the asserts
    printf("%s\n", debug_message);
    print_heap();
    exit(0);
}
#endif

#endif



#define MIN_BINARY_TREE_BLOCKSIZE (20)

#define BLACK (0)
#define RED (1)

static uint64_t redblack_tree_root_node = NIL;

/* ------------------------------------- */
/*  Operations for Tree Block Structure  */
/* ------------------------------------- */

static uint32_t get_redblack_tree_prev(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 4);
}

static uint32_t get_redblack_tree_left(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 8);
}

static uint32_t get_redblack_tree_right(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 12);
}

static uint32_t get_redblack_tree_color(uint64_t header_vaddr)
{
    if (header_vaddr == NIL)
    {
        // default BLACK
        return BLACK;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return ((header_value >> 1) & 0x1);
}

static void set_redblack_tree_prev(uint64_t header_vaddr, uint64_t prev_vaddr)
{
    set_field32_block_ptr(header_vaddr, prev_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 4);
}

static void set_redblack_tree_left(uint64_t header_vaddr, uint64_t left_vaddr)
{
    set_field32_block_ptr(header_vaddr, left_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 8);
}

static void set_redblack_tree_right(uint64_t header_vaddr, uint64_t right_vaddr)
{
    set_field32_block_ptr(header_vaddr, right_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 12);
}

static void set_redblack_tree_color(uint64_t header_vaddr, uint32_t color)
{
    if (header_vaddr == NIL)
    {
        return;
    }

    assert(color == BLACK || color == RED);
    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    *(uint32_t *)&heap[header_vaddr] &= 0xFFFFFFFD;
    *(uint32_t *)&heap[header_vaddr] |= ((color & 0x1) << 1);
}

/* ------------------------------------- */
/*  Operations for Red-Black Tree        */
/* ------------------------------------- */

static void redblack_tree_insert(uint64_t tree_root, uint64_t node_ptr)
{
    // TODO: implement insert
}

static void redblack_tree_delete(uint64_t tree_root, uint64_t node_ptr)
{
    // TODO: implement delete
}

static void redblack_tree_search(uint64_t tree_root, uint32_t size)
{
    // TODO: implement search
}

/* ------------------------------------- */
/*  For Debugging                        */
/* ------------------------------------- */


/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

static int internal_heap_init()
{
    // TODO
    return 0;
}

static uint64_t internal_malloc(uint32_t size)
{
    // TODO
    return 0;
}

static void internal_free(uint64_t payload_addr)
{
    // TODO
}