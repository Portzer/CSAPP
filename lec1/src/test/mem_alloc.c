//
// Created by M on 2024/7/15.
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int heap_init();
uint64_t mem_alloc(uint32_t size);
void mem_free(uint64_t vaddr);

static void check_heap_correctness();

static int implicit_list_heap_init();
static uint64_t implicit_list_mem_alloc(uint32_t size);
static void implicit_free_list_mem_free(uint64_t payload_vaddr);

static int explicit_free_list_heap_init();
static uint64_t explicit_free_list_mem_alloc(uint32_t size);
static void explicit_free_list_mem_free(uint64_t payload_vaddr);

static int binary_tree_heap_init();
static uint64_t binary_tree_mem_alloc(uint32_t size);
static void binary_tree_mem_free(uint64_t payload_vaddr);

// to allocate one physical page for heap
static uint32_t extend_heap(uint32_t size);




void os_syscall_brk()
{
    // an empty function
}

uint64_t heap_start_vaddr = 0;  // for unit test convenience
uint64_t heap_end_vaddr = 4096 ;

#define MIN_EXPLICIT_FREE_LIST_BLOCKSIZE (16)
#define HEAP_MAX_SIZE (4096 * 8)
#define MIN_IMPLICIT_FREE_LIST_BLOCKSIZE (8)
uint8_t heap[HEAP_MAX_SIZE];
static uint64_t explicit_list_head = 0;
static uint32_t explicit_list_counter = 0;

static uint64_t round_up(uint64_t x, uint64_t n);

static uint32_t get_block_size(uint64_t header_vaddr);
static void set_block_size(uint64_t header_vaddr, uint32_t block_size);

static uint32_t get_allocated(uint64_t header_vaddr);
static void set_allocated(uint64_t header_vaddr, uint32_t allocated);

static int is_first_block(uint64_t vaddr);
static int is_last_block(uint64_t vaddr);

static uint64_t get_payload_addr(uint64_t vaddr);
static uint64_t get_header_addr(uint64_t vaddr);
static uint64_t get_tail_addr(uint64_t vaddr);

static uint64_t get_next_header(uint64_t vaddr);
static uint64_t get_prev_header(uint64_t vaddr);


static uint64_t get_prologue();
static uint64_t get_epilogue();

static uint64_t get_first_block();
static uint64_t get_last_block();
static uint32_t extend_heap(uint32_t size);




static uint64_t round_up(uint64_t x, uint64_t n)
{
    return n * ((x + n - 1) / n);
}

static uint64_t get_payload_addr(uint64_t vaddr){

    return round_up(vaddr, 8);
}

static uint64_t get_prologue(){

    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    return heap_start_vaddr + 4;
}

static uint64_t get_epilogue(){

    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    return heap_end_vaddr - 4;
}

static uint32_t extend_heap(uint32_t size){

    size = round_up((uint64_t) size, 4096);

    if (heap_end_vaddr - heap_start_vaddr + size <= HEAP_MAX_SIZE) {

        os_syscall_brk();

        heap_end_vaddr += size;

    } else {
        return 0;
    }

    uint64_t epilogue = get_epilogue();

    set_allocated(epilogue, 0);
    set_block_size(epilogue, size);

    return size;
}

static uint64_t get_first_block(){

    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    return get_prologue() + 8;
}


static uint64_t get_last_block(){

    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    uint64_t epilogue_header = get_epilogue();
    uint64_t last_block_tail = epilogue_header - 4;
    uint32_t last_block_size = get_block_size(last_block_tail);

    uint64_t last_block_header = epilogue_header - last_block_size;

    assert(get_first_block() <= last_block_header);
    return last_block_header;
}


static int is_first_block(uint64_t vaddr){

    if (vaddr == 0) {
        return 0;
    }

    assert(get_first_block() <= vaddr && vaddr < get_epilogue());
    assert((vaddr & 0x3) == 0);


    uint64_t header_value = get_header_addr(vaddr);

    if (header_value == get_first_block()) {
        return 1;
    }
    return 0;

}

static int is_last_block(uint64_t vaddr){

    if (vaddr == 0) {
        return 0;
    }

    assert(get_prologue() <= vaddr && vaddr < get_epilogue());
    assert((vaddr & 0x3) == 0);


    uint64_t header_addr = get_header_addr(vaddr);
    uint32_t block_size = get_block_size(header_addr);
    if (header_addr + block_size == get_epilogue()) {
        return 1;
    }
    return 0;
}


static uint64_t get_tail_addr(uint64_t vaddr){

    if (vaddr == 0)
    {
        return 0;
    }
    assert(get_first_block() <= vaddr && vaddr < get_epilogue());
    assert((vaddr & 0x3) == 0);

    uint64_t header_addr = get_header_addr(vaddr);
    uint32_t block_size = get_block_size(header_addr);
    uint64_t tail_addr = header_addr + block_size - 4;

    assert(get_first_block() <= tail_addr && tail_addr < get_epilogue());

    return tail_addr;
}





static int check_block(uint64_t header_vaddr)
{
    // rule 1: block[0] ==> A/F
    // rule 2: block[-1] ==> A/F
    // rule 3: block[i] == A ==> block[i-1] == A/F && block[i+1] == A/F
    // rule 4: block[i] == F ==> block[i-1] == A && block[i+1] == A
    // these 4 rules ensures that
    // adjacent free blocks are always merged together
    // henceforth external fragmentation are minimized

    assert(header_vaddr % 8 == 4);

    if (get_allocated(header_vaddr) == 1)
    {
        // applies rule 3
        return 1;
    }
    uint32_t prev_allocated = 1;
    uint32_t next_allocated = 1;

    if (header_vaddr == heap_start_vaddr)
    {
        // the first block. there is no prev block
        // applies rule 1
        prev_allocated = 1;
    }
    else
    {
        prev_allocated = get_allocated(get_prev_header(header_vaddr));
    }

    if (is_last_block(header_vaddr) == 1)
    {
        // the last block. there is no next block
        // applies rule 2
        next_allocated = 1;
    }
    else
    {
        next_allocated = get_allocated(get_next_header(header_vaddr));

    }
    if (prev_allocated == 1 && next_allocated == 1)
    {
        return 1;
    }
    return 0;
}

static uint64_t get_header_addr(uint64_t vaddr){

    return get_payload_addr(vaddr) - 4;
}


static uint32_t get_block_size(uint64_t header_value){

    if (header_value == 0)
        return 0;

    assert(get_prologue() <= header_value && header_value <= get_epilogue());
    assert((header_value & 0x3) ==  0x0);

    uint32_t block_size = *(uint32_t *) &heap[header_value];
    return (block_size & 0xFFFFFFF8);
}


static void set_block_size(uint64_t header_value, uint32_t block_size){

    if (header_value == 0)
        return;


    assert(get_prologue() <= header_value && header_value <= get_epilogue());
    assert((header_value & 0x3) == 0);
    assert((block_size & 0x7) == 0);


    *(uint32_t *) &heap[header_value] &= 0x00000007;
    *(uint32_t *) &heap[header_value] |= block_size;
}


static void set_allocated(uint64_t header_value, uint32_t allocated){

    if (header_value == 0) {
        return;
    }

    assert(get_prologue() <= header_value && header_value <= get_epilogue());
    assert((header_value & 0x3) == 0);

    *(uint32_t *) &heap[header_value] &= 0xFFFFFFF8;
    *(uint32_t *) &heap[header_value] |= (allocated & 0x1);
}



static uint32_t get_allocated(uint64_t header_value){

    if (header_value == 0) {
        return 0;
    }

    assert(heap_start_vaddr <=  header_value && header_value <= heap_end_vaddr);
    assert((header_value & 0x3) == 0);

    uint32_t block_size = *(uint32_t *) &heap[header_value];
    return (block_size & 0x1);
}


static uint64_t get_next_header(uint64_t vaddr) {

    if (vaddr == 0 || is_last_block(vaddr)) {
        return 0;
    }

    assert(get_first_block()<= vaddr && vaddr <get_last_block());

    uint64_t head_value = get_header_addr(vaddr);

    uint32_t block_size = get_block_size(head_value);

    uint64_t next_header_value = head_value + block_size;

    assert(get_first_block()< next_header_value && next_header_value <= get_last_block());

    return next_header_value;

}

static uint64_t get_prev_header(uint64_t vaddr){

    if (vaddr == 0 || is_first_block(vaddr)) {
        return 0;
    }

    assert(get_first_block()< vaddr && vaddr <= get_last_block());

    uint64_t head_value = get_header_addr(vaddr);

    uint64_t prev_tail_value = head_value - 4;

    uint32_t prev_block_size = get_block_size(prev_tail_value);

    uint64_t prev_header_value = prev_tail_value - prev_block_size;

    assert(get_first_block()<= prev_header_value && prev_header_value  < get_last_block());
    assert(*(uint32_t *)&heap[prev_header_value]== (*(uint32_t *)&heap[prev_tail_value]));
    return prev_header_value;
}


static int implicit_list_heap_init(){

    // heap clear
    for (int i = 0; i < HEAP_MAX_SIZE / 8; ++i) {
        *(uint64_t *) &heap[i] = 0;
    }

    heap_start_vaddr = 0;
    heap_end_vaddr = 4096;

    // prologue
    uint64_t prologue_header = get_prologue();
    set_allocated(prologue_header, 1);
    set_block_size(prologue_header, 8);

    uint64_t prologue_tail = prologue_header + 4;
    set_allocated(prologue_tail, 1);
    set_block_size(prologue_tail, 8);

    //epilogue
    uint64_t epilogue_header = get_epilogue();
    set_allocated(epilogue_header, 1);
    set_block_size(epilogue_header, 4);

    // block header
    uint64_t first_block_header = get_first_block();
    set_allocated(first_block_header, 0);
    set_block_size(first_block_header, heap_end_vaddr - 8 - 4 - 4);

    // block tail
    uint64_t first_block_tail = get_tail_addr(first_block_header);
    set_allocated(first_block_tail, 0);
    set_block_size(first_block_tail, heap_end_vaddr - 8 - 4 - 4);

    return 1;
}

int heap_init(){

    #ifdef IMPLICIT_FREE_LIST
    return implicit_free_list_heap_init();
    #endif

    #ifdef EXPLICIT_FREE_LIST
    return explicit_free_list_heap_init();
    #endif

    #ifdef FREE_BINARY_TREE
    return binary_tree_heap_init();
    #endif
    return 0;
}


static void check_heap_correctness(){

    uint64_t first_block = get_first_block();
    uint64_t last_block = get_last_block();
    int free_count = 0;
    uint64_t block = first_block;
    while (block != 0 && block < last_block) {

        assert(block % 8 == 4);
        assert(first_block <= block && block <= last_block);
        assert(*(uint32_t*)&heap[block] ==*(uint32_t*)&*(uint32_t*)&heap[get_tail_addr(block)]);

        uint32_t allocated = get_allocated(block);

        if (allocated == 1) {
            free_count++;
        } else {
            free_count = 0;
        }
        assert(free_count <= 1);
        block = get_next_header(block);
    }
}


uint64_t try_alloc_with_splitting(uint64_t addr, uint32_t req_block_size) {

    uint64_t allocated = get_allocated(addr);
    uint64_t block_size = get_block_size(addr);

    if (allocated == 0) {

        if (block_size - req_block_size >= MIN_IMPLICIT_FREE_LIST_BLOCKSIZE) {

            uint64_t next_header_addr = get_next_header(addr);
            set_block_size(next_header_addr, block_size - req_block_size);
            set_allocated(next_header_addr, 0);

            uint64_t next_tail_addr = get_tail_addr(next_header_addr);
            set_block_size(next_tail_addr, block_size - req_block_size);
            set_allocated(next_tail_addr, 0);

            set_block_size(addr, req_block_size);
            set_allocated(addr, 1);

            uint64_t tail_addr = get_tail_addr(addr);
            set_block_size(tail_addr, req_block_size);
            set_allocated(tail_addr, 1);

            return get_payload_addr(addr);
        }else{

            set_block_size(addr, block_size);
            set_allocated(addr, 1);
            return get_payload_addr(addr);
        }
    }
    return 0;
}


static uint64_t try_extend_heap_to_alloc(uint64_t block_size){

    uint64_t last_block_addr = get_last_block();
    uint64_t last_block_size = get_block_size(last_block_addr);
    uint64_t last_allocated = get_allocated(last_block_addr);

    uint32_t OS_size = block_size;
    if (last_allocated == 0) {
        OS_size -= last_block_size;
    }
    uint64_t old_epilogue_addr = get_epilogue();
    uint32_t OS_allocated_size = extend_heap(OS_size);

    uint64_t header_addr = 0;

    if (OS_allocated_size != 0) {

        if (last_allocated == 1) {

            set_allocated(old_epilogue_addr, 0);
            set_block_size(old_epilogue_addr, OS_allocated_size);

            uint64_t tail_addr = get_tail_addr(old_epilogue_addr);
            set_allocated(tail_addr, 0);
            set_block_size(tail_addr, OS_allocated_size);

            header_addr = old_epilogue_addr;

        } else if (last_allocated == 0) {

            set_block_size(last_block_addr, OS_allocated_size + last_block_size);
            set_allocated(last_block_addr, 0);


            uint64_t tail_addr = get_tail_addr(old_epilogue_addr);
            set_allocated(tail_addr, 0);
            set_block_size(tail_addr, OS_allocated_size);

            header_addr = last_block_addr;
        }
    }

    uint64_t payload_vaddr = try_alloc_with_splitting(header_addr, block_size);

    if (header_addr != 0)
    {
        #ifdef DEBUG_MALLOC
        check_heap_correctness();
        #endif
        return payload_vaddr;
    }
    return 0;
}

static uint64_t implicit_list_mem_alloc(uint32_t size){

    assert(size > 0 && size <= HEAP_MAX_SIZE - 4 - 8 - 4);
    uint64_t block_size = round_up(size, 8) + 8;
    uint64_t addr = get_first_block();

    while (addr != 0 && addr < get_epilogue()) {

        uint64_t payload_addr = try_alloc_with_splitting(addr, block_size);

        if (payload_addr != 0) {

        #ifdef DEBUG_MALLOC
            check_heap_correctness();
        #endif

            return payload_addr;
        }else{
            addr = get_next_header(addr);
        }
    }
    return try_extend_heap_to_alloc(block_size);
}

static uint64_t get_block_ptr(uint64_t header_vaddr , uint32_t offset)
{
    assert(get_first_block() <= header_vaddr && header_vaddr <= get_last_block());
    assert(header_vaddr % 8 == 4);
    assert(get_block_size(header_vaddr) >= MIN_IMPLICIT_FREE_LIST_BLOCKSIZE);

    assert(offset % 4 == 0);

    uint32_t vaddr_32 = *(uint32_t *)&heap[header_vaddr + offset];
    return (uint64_t)vaddr_32;
}

static void set_block_ptr(uint64_t header_vaddr, uint64_t block_ptr, uint32_t offset)
{
    assert(get_first_block() <= header_vaddr && header_vaddr <= get_last_block());
    assert(header_vaddr % 8 == 4);
    assert(get_block_size(header_vaddr) >= MIN_IMPLICIT_FREE_LIST_BLOCKSIZE);

    assert(get_first_block() <= block_ptr && block_ptr <= get_last_block());
    assert(block_ptr % 8 == 4);
    assert(get_block_size(block_ptr) >= MIN_IMPLICIT_FREE_LIST_BLOCKSIZE);

    assert(offset % 4 == 0);

    assert((block_ptr >> 32) == 0);
    *(uint32_t *)(&heap[header_vaddr+offset]) = block_ptr;
}




static uint64_t get_free_prev(uint64_t header_vaddr)
{
    return get_block_ptr(header_vaddr, 4);
}

static uint64_t get_free_next(uint64_t header_vaddr)
{
    return get_block_ptr(header_vaddr, 8);
}

static void set_free_prev(uint64_t header_vaddr, uint64_t prev_vaddr)
{
    set_block_ptr(header_vaddr, prev_vaddr, 4);
}

static void set_free_next(uint64_t header_vaddr, uint64_t next_vaddr)
{

    set_block_ptr(header_vaddr, next_vaddr, 8);
}


static void explicit_list_insert(uint64_t * header_ptr,uint32_t *explicit_counter,uint64_t block){

    assert(get_first_block() <= block && block <= get_last_block());
    assert(block % 8 == 4);
    assert(get_block_size(block) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    uint64_t free_header = *header_ptr;
    uint64_t free_tail = get_free_prev(free_header);

    if ((*header_ptr) == 0 && (*explicit_counter) == 0) {

        set_free_prev(free_tail, block);
        set_free_next(block, free_header);

        (*explicit_counter)++;
        (*header_ptr) = block;

        return;
    }

    set_free_next(free_tail, block);
    set_free_prev(block, free_tail);

    set_free_next(free_header, block);
    set_free_next(block, free_header);

    (*header_ptr) = block;
    (*explicit_counter) += 1;
}


static void explicit_list_delete(uint64_t *header_ptr,uint32_t *explicit_counter,uint64_t block){

    assert(get_first_block() <= block && block <= get_last_block());
    assert(block % 8 == 4);
    assert(get_block_size(block) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    if ((*header_ptr) == 0 && (*explicit_counter) == 0) {
        assert((*header_ptr) == 0);
        assert((*explicit_counter) == 0);
        return;
    }

    if ((*explicit_counter) == 1) {

        assert(get_free_next(*header_ptr) == get_free_prev(block));
        assert(get_free_prev(*header_ptr) == get_free_next(block));
        (*header_ptr) = 0;
        (*explicit_counter) = 0;
        return;
    }

    uint64_t prev = get_free_prev(block);
    uint64_t next = get_free_next(block);

    if ((*header_ptr) == block) {

        (*header_ptr) = next;
    }

    set_free_next(block, next);
    set_free_prev(next, prev);

    (*explicit_counter) -= 1;
}


static int explicit_free_list_heap_init(){

    if (implicit_list_heap_init() == 0) {
        return 0;
    }

    uint64_t first_block = get_first_block();

    explicit_list_insert(&explicit_list_head, &explicit_list_counter, first_block);

    return 1;
}




static uint64_t explicit_free_list_mem_alloc(uint32_t size){

    assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

    uint64_t payload_vaddr = 0;

    uint32_t request_block_size = round_up(size, 8) + 4 + 4;
    request_block_size = request_block_size < MIN_EXPLICIT_FREE_LIST_BLOCKSIZE ?
                        MIN_EXPLICIT_FREE_LIST_BLOCKSIZE : request_block_size;


    uint64_t temp = explicit_list_counter;
    uint64_t b = explicit_list_head;

    for (int i = 0; i < temp; ++i) {

        uint32_t old_block_size = get_block_size(b);

        uint64_t payload_addr = try_alloc_with_splitting(b, request_block_size);

        if (payload_addr != 0) {

            uint64_t header_addr = get_header_addr(payload_vaddr);
            uint64_t new_block_size = get_block_size(header_addr);

            explicit_list_delete(&explicit_list_head, &explicit_list_counter, header_addr);

            assert(new_block_size <= old_block_size);

            if (new_block_size < old_block_size) {

                uint64_t next_header_addr = get_next_header(header_addr);

                explicit_list_insert(&explicit_list_head, &explicit_list_counter, next_header_addr);

            }
            return payload_vaddr;
        } else {
            b = get_free_next(b);
        }
    }

    uint64_t old_block_addr = get_last_block();

    uint64_t last_allocated = get_allocated(old_block_addr);

    if (last_allocated == 0) {
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, old_block_addr);
    }
    payload_vaddr = try_extend_heap_to_alloc(request_block_size);

    uint64_t payload_header_addr = get_header_addr(payload_vaddr);

    uint64_t playload_block_addr = get_next_header(payload_header_addr);

    if (get_allocated(payload_vaddr == 0)) {
        explicit_list_insert(&explicit_list_head, &explicit_list_counter, playload_block_addr);
    }

    return payload_vaddr;
}



static uint64_t merge_blocks_as_free(uint64_t low, uint64_t high){

    assert(low < high);
    assert(low % 8 == 4 && high % 8 == 4);
    assert(get_first_block() <= low && low <= get_last_block());
    assert(get_first_block() <= high && high <= get_last_block());
    assert(get_next_header(low) == high);
    assert(get_prev_header(high) == low);

    uint32_t block_size = get_block_size(low) + get_block_size(high);
    set_block_size(low, block_size);
    set_allocated(low, 0);

    uint64_t tail_addr = get_tail_addr(high);
    set_block_size(tail_addr, block_size);
    set_allocated(tail_addr, 0);
    return low;
}

static void implicit_list_mem_free(uint64_t payload_vaddr){

    if (payload_vaddr == 0) {
        return;
    }
    assert(get_first_block() < payload_vaddr && payload_vaddr < get_epilogue());

    uint64_t header_vaddr = get_header_addr(payload_vaddr);
    assert(get_allocated(header_vaddr) == 1);
    uint64_t prev_header_vaddr = get_prev_header(header_vaddr);
    uint64_t next_header_vaddr = get_next_header(header_vaddr);

    uint32_t prev_block_size = get_block_size(prev_header_vaddr);
    uint32_t next_block_size = get_block_size(next_header_vaddr);

    uint64_t prev_allocated = get_allocated(prev_header_vaddr);
    uint64_t next_allocated = get_allocated(next_header_vaddr);


    if (prev_allocated == 1 && next_allocated == 1) {

        set_allocated(header_vaddr, 0);
        uint64_t tail_addr = get_tail_addr(header_vaddr);
        set_allocated(tail_addr, 0);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    } else if (prev_allocated == 0 && next_allocated == 0) {

        header_vaddr = merge_blocks_as_free(prev_header_vaddr, header_vaddr);
        merge_blocks_as_free(header_vaddr, next_header_vaddr);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    } else if (prev_allocated == 0 && next_allocated == 1) {

        merge_blocks_as_free(prev_header_vaddr, header_vaddr);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    } else {

        merge_blocks_as_free(header_vaddr, prev_header_vaddr);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }

}


static void explicit_list_mem_free(uint64_t payload_vaddr)
{
    if (payload_vaddr == 0)
    {
        return;
    }

    assert(get_first_block() < payload_vaddr && payload_vaddr < get_epilogue());
    assert((payload_vaddr & 0x7) == 0x0);

    uint64_t req = get_header_addr(payload_vaddr);
    uint64_t req_footer = get_tail_addr(req);

    uint32_t req_allocated = get_allocated(req);
    uint32_t req_block_size = get_block_size(req);
    assert(req_allocated == 1);

    uint64_t next = get_next_header(req);
    uint64_t prev = get_prev_header(req);

    uint32_t next_allocated = get_allocated(next);  // for req last, 1
    uint32_t prev_allocated = get_allocated(prev);  // for req first, 1

    if (next_allocated == 1 && prev_allocated == 1)
    {
        // case 1: *A(A->F)A*
        // ==> *AFA*
        set_allocated(req, 0);
        set_allocated(req_footer, 0);

        explicit_list_insert(&explicit_list_head, &explicit_list_counter, req);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == 0 && prev_allocated == 1)
    {
        // case 2: *A(A->F)FA
        // ==> *AFFA ==> *A[FF]A merge current and next
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, next);

        uint64_t one_free  = merge_blocks_as_free(req, next);

        explicit_list_insert(&explicit_list_head, &explicit_list_counter, one_free);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == 1 && prev_allocated == 0)
    {
        // case 3: AF(A->F)A*
        // ==> AFFA* ==> A[FF]A* merge current and prev
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, prev);

        uint64_t one_free  = merge_blocks_as_free(prev, req);

        explicit_list_insert(&explicit_list_head, &explicit_list_counter, one_free);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == 0 && prev_allocated == 0)
    {
        // case 4: AF(A->F)FA
        // ==> AFFFA ==> A[FFF]A merge current and prev and next
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, prev);
        explicit_list_delete(&explicit_list_head, &explicit_list_counter, next);

        uint64_t one_free = merge_blocks_as_free(merge_blocks_as_free(prev, req), next);

        explicit_list_insert(&explicit_list_head, &explicit_list_counter, one_free);
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





uint64_t mem_alloc(uint32_t size){
    #ifdef IMPLICIT_FREE_LIST
    return implicit_list_mem_alloc(size);
    #endif

    #ifdef EXPLICIT_FREE_LIST
    return explicit_list_mem_alloc(size);
    #endif

    #ifdef FREE_BINARY_TREE
    return binary_tree_mem_alloc(size);
    #endif
    return 0;
}

void mem_free(uint64_t payload_vaddr)
{
    #ifdef IMPLICIT_FREE_LIST
    implicit_list_mem_free(payload_vaddr);
    #endif

    #ifdef EXPLICIT_FREE_LIST
    explicit_list_mem_free(payload_vaddr);
    #endif

    #ifdef FREE_BINARY_TREE
    binary_tree_mem_free(payload_vaddr);
    #endif
}




static void test_roundup()
{
    printf("Testing round up ...\n");

    for (int i = 0; i < 100; ++ i)
    {
        for (int j = 1; j <= 8; ++ j)
        {
            uint32_t x = i * 8 + j;
            assert(round_up(x, 8) == (i + 1) * 8);
        }
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

/*  hex table
    0       0000    *
    1       0001    *
    2       0010
    3       0011
    4       0100
    5       0101
    6       0110
    7       0111
    8       1000    *
    9       1001    *
    a   10  1010
    b   11  1011
    c   12  1100
    d   13  1101
    e   14  1110
    f   15  1111
 */
static void test_get_blocksize_allocated()
{
    printf("Testing getting block size from header ...\n");

    for (int i = get_prologue(); i <= get_epilogue(); i += 4)
    {
        *(uint32_t *)&heap[i] = 0x1234abc0;
        assert(get_block_size(i) == 0x1234abc0);
        assert(get_allocated(i) == 0);

        *(uint32_t *)&heap[i] = 0x1234abc1;
        assert(get_block_size(i) == 0x1234abc0);
        assert(get_allocated(i) == 1);

        *(uint32_t *)&heap[i] = 0x1234abc8;
        assert(get_block_size(i) == 0x1234abc8);
        assert(get_allocated(i) == 0);

        *(uint32_t *)&heap[i] = 0x1234abc9;
        assert(get_block_size(i) == 0x1234abc8);
        assert(get_allocated(i) == 1);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_set_blocksize_allocated()
{
    printf("Testing setting block size to header ...\n");

    for (int i = get_prologue(); i <= get_epilogue(); i += 4)
    {
        set_block_size(i, 0x1234abc0);
        set_allocated(i, 0);
        assert(get_block_size(i) == 0x1234abc0);
        assert(get_allocated(i) == 0);

        set_block_size(i, 0x1234abc0);
        set_allocated(i, 1);
        assert(get_block_size(i) == 0x1234abc0);
        assert(get_allocated(i) == 1);

        set_block_size(i, 0x1234abc8);
        set_allocated(i, 0);
        assert(get_block_size(i) == 0x1234abc8);
        assert(get_allocated(i) == 0);

        set_block_size(i, 0x1234abc8);
        set_allocated(i, 1);
        assert(get_block_size(i) == 0x1234abc8);
        assert(get_allocated(i) == 1);

    }

    // set the block size of last block
    for (int i = 2; i < 100; ++ i)
    {
        uint32_t blocksize = i * 8;
        uint64_t addr = get_epilogue() - blocksize;   // + 4 for the virtual footer in next page
        set_block_size(addr, blocksize);
        assert(get_block_size(addr) == blocksize);
        assert(is_last_block(addr) == 1);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_get_header_payload_addr()
{
    printf("Testing getting header or payload virtual addresses ...\n");

    uint64_t header_addr, payload_addr;
    for (int i = get_payload_addr(get_first_block()); i < get_epilogue(); i += 8)
    {
        payload_addr = i;
        header_addr = payload_addr - 4;

        assert(get_payload_addr(header_addr) == payload_addr);
        assert(get_payload_addr(payload_addr) == payload_addr);

        assert(get_header_addr(header_addr) == header_addr);
        assert(get_header_addr(payload_addr) == header_addr);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_get_next_prev()
{
    printf("Testing linked list traversal ...\n");

    srand(123456);

    // let me setup the heap first
    heap_init();

    uint64_t h = get_first_block();
    uint64_t f = 0;

    uint32_t collection_blocksize[1000];
    uint32_t collection_allocated[1000];
    uint32_t collection_headeraddr[1000];
    int counter = 0;

    uint32_t allocated = 1;
    uint64_t epilogue = get_epilogue();
    while (h < epilogue)
    {
        uint32_t blocksize = 8 * (1 + rand() % 16);
        if (epilogue - h <= 64)
        {
            blocksize = epilogue - h;
        }

        if (allocated == 1 && (rand() % 3) >= 1)
        {
            // with previous allocated, 2/3 possibilities to be free
            allocated = 0;
        }
        else
        {
            allocated = 1;
        }

        collection_blocksize[counter] = blocksize;
        collection_allocated[counter] = allocated;
        collection_headeraddr[counter] = h;
        counter += 1;

        set_block_size(h, blocksize);
        set_allocated(h, allocated);

        f = h + blocksize - 4;
        set_block_size(f, blocksize);
        set_allocated(f, allocated);

        h = h + blocksize;
    }

    // check get_next
    h = get_first_block();
    int i = 0;
    while (h != 0 && h < get_epilogue())
    {
        assert(i <= counter);
        assert(h == collection_headeraddr[i]);
        assert(get_block_size(h) == collection_blocksize[i]);
        assert(get_allocated(h) == collection_allocated[i]);

        h = get_next_header(h);
        i += 1;
    }

    check_heap_correctness();

    // check get_prev
    h = get_last_block();
    i = counter - 1;
    while (h != 0 && get_first_block() <= h)
    {
        assert(0 <= i);
        assert(h == collection_headeraddr[i]);
        assert(get_block_size(h) == collection_blocksize[i]);
        assert(get_allocated(h) == collection_allocated[i]);

        h = get_prev_header(h);
        i -= 1;
    }

    printf("\033[32;1m\tPass\033[0m\n");
}



int main()
{
    test_roundup();
    test_get_blocksize_allocated();
    test_set_blocksize_allocated();
    test_get_header_payload_addr();
    test_get_next_prev();


    return 0;
}

