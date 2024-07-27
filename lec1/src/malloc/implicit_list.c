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

#ifdef IMPLICIT_FREE_LIST

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



#define MIN_IMPLICIT_FREE_LIST_BLOCKSIZE (8)

/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

static int internal_heap_init()
{
    // reset all to 0
    for (int i = 0; i < HEAP_MAX_SIZE / 8; i += 8)
    {
        *(uint64_t *)&heap[i] = 0;
    }

    // heap_start_vaddr is the starting address of the first block
    // the payload of the first block is 8B aligned ([8])
    // so the header address of the first block is [8] - 4 = [4]
    heap_start_vaddr = 0;
    heap_end_vaddr = 4096;

    // set the prologue block
    uint64_t prologue_header = get_prologue();
    set_block_size(prologue_header, 8);
    set_allocated(prologue_header, ALLOCATED);

    uint64_t prologue_footer = prologue_header + 4;
    set_block_size(prologue_footer, 8);
    set_allocated(prologue_footer, ALLOCATED);

    // set the epilogue block
    // it's a footer only
    uint64_t epilogue = get_epilogue();
    set_block_size(epilogue, 0);
    set_allocated(epilogue, ALLOCATED);

    // set the block size & allocated of the only regular block
    uint64_t first_header = get_first_block();
    set_block_size(first_header, 4096 - 4 - 8 - 4);
    set_allocated(first_header, FREE);

    uint64_t first_footer = get_tail(first_header);
    set_block_size(first_footer, 4096 - 4 - 8 - 4);
    set_allocated(first_footer, FREE);

#ifdef DEBUG_MALLOC
    // like a try-catch
    signal(SIGABRT, &on_sigabrt);
#endif

    return 1;
}

// size - requested payload size
// return - the virtual address of payload
static uint64_t internal_malloc(uint32_t size)
{
    assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

    uint64_t payload_vaddr = NIL;
    uint32_t request_blocksize = round_up(size, 8) + 4 + 4;
    request_blocksize = request_blocksize < MIN_IMPLICIT_FREE_LIST_BLOCKSIZE ?
                        MIN_IMPLICIT_FREE_LIST_BLOCKSIZE : request_blocksize;

    uint64_t b = get_first_block();
    uint64_t epilogue = get_epilogue();

    // not thread safe
    while (b != NIL && b < epilogue)
    {
        payload_vaddr = try_alloc_with_splitting(b, request_blocksize, MIN_IMPLICIT_FREE_LIST_BLOCKSIZE);

        if (payload_vaddr != NIL)
        {
#ifdef DEBUG_MALLOC
            check_heap_correctness();
#endif
            return payload_vaddr;
        }
        else
        {
            // go to next block
            b = get_next_header(b);
        }
    }

    // when no enough free block for current heap
    // request a new free physical & virtual page from OS
    return try_extend_heap_to_alloc(request_blocksize, MIN_IMPLICIT_FREE_LIST_BLOCKSIZE);
}

static void internal_free(uint64_t payload_vaddr)
{
    if (payload_vaddr == NIL)
    {
        return;
    }

    assert(get_first_block() < payload_vaddr && payload_vaddr < get_epilogue());
    assert((payload_vaddr & 0x7) == 0x0);

    // request can be first or last block
    uint64_t req = get_header(payload_vaddr);
    uint64_t req_footer = get_tail(req); // for last block, it's 0

    uint32_t req_allocated = get_allocated(req);
    uint32_t req_blocksize = get_block_size(req);
    assert(req_allocated == ALLOCATED); // otherwise it's free twice

    // block starting address of next & prev blocks
    uint64_t next = get_next_header(req);    // for req last block, it's 0
    uint64_t prev = get_prev_header(req);    // for req first block, it's 0

    uint32_t next_allocated = get_allocated(next);  // for req last, 1
    uint32_t prev_allocated = get_allocated(prev);  // for req first, 1

    if (next_allocated == ALLOCATED && prev_allocated == ALLOCATED)
    {
        // case 1: *A(A->F)A*
        // ==> *AFA*
        set_allocated(req, FREE);
        set_allocated(req_footer, FREE);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == FREE && prev_allocated == ALLOCATED)
    {
        // case 2: *A(A->F)FA
        // ==> *AFFA ==> *A[FF]A merge current and next
        merge_blocks_as_free(req, next);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == ALLOCATED && prev_allocated == FREE)
    {
        // case 3: AF(A->F)A*
        // ==> AFFA* ==> A[FF]A* merge current and prev
        merge_blocks_as_free(prev, req);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == FREE && prev_allocated == FREE)
    {
        // case 4: AF(A->F)FA
        // ==> AFFFA ==> A[FFF]A merge current and prev and next
        uint64_t merged = merge_blocks_as_free(prev, req);
        merge_blocks_as_free(merged, next);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else
    {
#ifdef DEBUG_MALLOC
        printf("exception for free\n");
        exit(0);
#endif
    }
}