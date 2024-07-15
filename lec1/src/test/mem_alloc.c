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

uint64_t heap_start_vaddr = 4;  // for unit test convenience
uint64_t heap_end_vaddr = 4096 - 1;

#define HEAP_MAX_SIZE (4096 * 8)
uint8_t heap[HEAP_MAX_SIZE];

static uint64_t round_up(uint64_t x, uint64_t n);

static uint32_t get_block_size(uint64_t header_vaddr);
static void set_block_size(uint64_t header_vaddr, uint32_t block_size);

static uint32_t get_allocated(uint64_t header_vaddr);
static void set_allocated(uint64_t header_vaddr, uint32_t allocated);

static int is_last_block(uint64_t vaddr);

static uint64_t get_payload_addr(uint64_t vaddr);
static uint64_t get_header_addr(uint64_t vaddr);

static uint64_t get_next_header(uint64_t vaddr);
static uint64_t get_prev_header(uint64_t vaddr);

// Round up to next multiple of n:
// if (x == k * n)
// return x
// else, x = k * n + m and m < n
// return (k + 1) * n
static uint64_t round_up(uint64_t x, uint64_t n)
{
    return n * ((x + n - 1) / n);
}

static uint64_t get_payload_addr(uint64_t vaddr){

    return round_up(vaddr, 8);
}


static int is_last_block(uint64_t vaddr){

    assert(heap_start_vaddr <= vaddr && vaddr <= heap_end_vaddr);
    assert((vaddr & 0x3) == 0);


    uint64_t header_value = get_header_addr(vaddr);
    uint32_t block_size = get_block_size(header_value);

    if (header_value + block_size == heap_end_vaddr + 1 + 4) {
        return 1;
    }
    return 0;
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

    #ifdef DEBUG_MALLOC
    #endif

    assert(heap_start_vaddr <= header_value && header_value <= heap_end_vaddr);
    assert((header_value & 0x3) ==  0x0);

    uint32_t block_size = *(uint32_t *) &heap[header_value];
    return (block_size & 0xFFFFFFF8);
}


static void set_block_size(uint64_t header_value, uint32_t block_size){

    assert(heap_start_vaddr <= header_value && header_value <= heap_end_vaddr);
    assert((header_value & 0x3) == 0);
    assert((block_size & 0x7) == 0);


    *(uint32_t *) &heap[header_value] &= 0x7;
    *(uint32_t *) &heap[header_value] |= block_size;
}


static void set_allocated(uint64_t header_value, uint32_t allocated){

    assert(heap_start_vaddr <= header_value && header_value <= heap_end_vaddr);
    assert((header_value & 0x3) == 0);

    *(uint32_t *) &heap[header_value] &= 0xFFFFFFF8;
    *(uint32_t *) &heap[header_value] |= (allocated & 0x1);
}



static uint32_t get_allocated(uint64_t header_value){

    assert(heap_start_vaddr <=  header_value && header_value <= heap_end_vaddr);
    assert((header_value & 0x3) == 0);

    uint32_t block_size = *(uint32_t *) &heap[header_value];
    return (block_size & 0x1);
}


static uint64_t get_next_header(uint64_t vaddr) {

    uint64_t head_value = get_header_addr(vaddr);

    uint32_t block_size = get_block_size(head_value);

    uint64_t next_header_value = head_value + block_size;

    assert(heap_start_vaddr <= next_header_value && next_header_value <= heap_end_vaddr);

    return next_header_value;

}


int heap_init()
{

    heap_start_vaddr = 4;

    set_block_size(heap_start_vaddr, 4096 - 8);
    set_allocated(heap_start_vaddr, 0);

    // we do not set footer for the last block in heap
    heap_end_vaddr = 4096 - 1;

    return 0;
}

static uint64_t get_prev_header(uint64_t vaddr) {

    uint64_t header_value = get_header_addr(vaddr);
    assert(header_value >= 16);

    uint64_t prev_tail_value = header_value - 4;

    uint32_t block_size = get_block_size(prev_tail_value);

    uint64_t prev_header_value = header_value - block_size;

    assert(heap_start_vaddr <= prev_header_value && prev_header_value <= heap_end_vaddr - 12);

    return prev_header_value;

}

uint64_t mem_alloc(uint32_t size){

    uint64_t b = heap_start_vaddr;

    uint64_t req_block_size = round_up(size, 8) + 4 + 4;

    while (b <=  heap_end_vaddr) {

        uint32_t block_size = get_block_size(b);

        uint32_t allocated = get_allocated(b);

        if (allocated == 0) {

            if (block_size >= req_block_size) {

                set_allocated(b, 1);
                set_block_size(b, req_block_size);

                uint64_t next_header_value = b + req_block_size;
                set_allocated(next_header_value, 0);
                set_block_size(next_header_value, block_size - req_block_size);
                return get_payload_addr(b);

            } else {
                set_allocated(b, 1);
                return get_payload_addr(b);
            }

        } else {
            b = get_next_header(b);
        }

    }

    return 0;
}


void mem_free(uint64_t vaddr) {

    assert(heap_start_vaddr <= vaddr && vaddr <= heap_end_vaddr);
    assert((vaddr && 0x7) == 0x0);

    uint64_t head_value = get_header_addr(vaddr);
    uint32_t block_size = get_block_size(head_value);
    uint32_t allocated = get_allocated(head_value);
    assert(allocated == 1);

    uint64_t next_header_value = get_next_header(vaddr);
    uint64_t prev_header_value = get_prev_header(vaddr);

    uint32_t next_allocated = get_allocated(next_header_value);
    uint32_t prev_allocated = get_allocated(prev_header_value);

    uint32_t next_block_size = get_block_size(next_header_value);
    uint32_t prev_block_size = get_block_size(prev_header_value);


    if (next_allocated == 1 && prev_allocated == 1) {

        set_allocated(head_value, 1);
        set_block_size(head_value, block_size);

    } else if (next_allocated == 1 && prev_allocated == 0) {

        //A F (F) A -> A F A
        set_allocated(prev_header_value, 1);
        set_block_size(prev_header_value, prev_block_size + block_size);
    } else if (next_allocated == 0 && prev_allocated == 1) {

        //A (F) F A -> A F A
        set_allocated(head_value, 1);
        set_block_size(head_value, next_block_size + block_size);
    } else {

        set_allocated(prev_header_value, 0);
        set_block_size(prev_header_value, block_size + prev_block_size + next_block_size);
    }

}


#ifdef DEBUG_MALLOC

void test_roundup(){

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
void test_get_block_size_allocated()
{
    printf("Testing getting block size from header ...\n");

    for (int i = 4; i <= 4096-1; i += 4)
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

void test_set_block_size_allocated()
{
    printf("Testing setting block size to header ...\n");

    for (int i = 4; i <= 4096-1; i += 4)
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
        uint32_t block_size = i * 8;
        uint64_t addr = 4096 + 4 - block_size;   // + 4 for the virtual footer in next page
        set_block_size(addr, block_size);
        assert(get_block_size(addr) == block_size);
        assert(is_last_block(addr) == 1);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

void test_get_header_payload_addr()
{
    printf("Testing getting header or payload virtual addresses ...\n");

    uint64_t header_addr, payload_addr;
    for (int i = 8; i <= 4096-1; i += 8)
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

void test_get_next_prev()
{
    printf("Testing linked list traversal ...\n");

    srand(123456);

    // let me setup the heap first
    uint64_t h = heap_start_vaddr;
    uint64_t f = 0;

    uint32_t prev_allocated = 1;    // dummy allocated

    uint32_t collection_block_size[1000];
    uint32_t collection_allocated[1000];
    uint32_t collection_headeraddr[1000];
    int counter = 0;

    while (h <= heap_end_vaddr)
    {
        uint32_t block_size = 8 * (1 + rand() % 16);
        if (heap_end_vaddr - h <= 64)
        {
            block_size = 4096 + 4 - h;
        }

        uint32_t allocated = 1;
        if (prev_allocated == 1 && (rand() & 0x1) == 1)
        {
            allocated = 0;
        }

        collection_block_size[counter] = block_size;
        collection_allocated[counter] = allocated;
        collection_headeraddr[counter] = h;

        set_block_size(h, block_size);
        set_allocated(h, allocated);
        if (is_last_block(h) == 0)
        {
            f = h + block_size - 4;
            set_block_size(f, block_size);
            set_allocated(f, allocated);
        }
        h = h + block_size;
        prev_allocated = allocated;
        counter += 1;
    }

    // check get_next
    h = heap_start_vaddr;
    int i = 0;
    while (is_last_block(h) == 0)
    {
        assert(i < counter);
        assert(h == collection_headeraddr[i]);
        assert(get_block_size(h) == collection_block_size[i]);
        assert(get_allocated(h) == collection_allocated[i]);
        assert(check_block(h) == 1);
        h = get_next_header(h);
        i += 1;
    }

    // check the last block
    assert(is_last_block(h));

    // check get_prev
    i = counter - 1;
    while (heap_end_vaddr <= h)
    {
        assert(0 <= i);
        assert(h == collection_headeraddr[i]);
        assert(get_block_size(h) == collection_block_size[i]);
        assert(get_allocated(h) == collection_allocated[i]);
        assert(check_block(h) == 1);

        h = get_prev_header(h);
        i -= 1;
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    test_roundup();
    test_get_block_size_allocated();
    test_set_block_size_allocated();
    test_get_header_payload_addr();
    test_get_next_prev();

    return 0;
}
#endif


