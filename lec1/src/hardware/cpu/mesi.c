//
// Created by M on 2024/6/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// states of MESI
typedef enum
{
    // exclusive modified: the only dirty copy
    MODIFIED,
    // exclusive clean: the only copy
    EXCLUSIVE,
    // shared clean: multiple processors may hold this exact same copy
    SHARED,
    // invalid copy of the physical address
    // the cache line may hold data of other physical address
    INVALID
} cachestate_t;

// struct of a MESI cache line
typedef struct
{
    cachestate_t    state;
    int             value;
} cacheline_t;

#ifndef NUM_PROCESSOR
#define NUM_PROCESSOR (2048)
#endif


cacheline_t cache[NUM_PROCESSOR];

int cache_value = 15213;



int check_states(){
    int m_count = 0;
    int e_count = 0;
    int s_count = 0;
    int i_count = 0;

    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        if (cache[i].state == MODIFIED)
        {
            m_count += 1;
        }
        else if (cache[i].state == EXCLUSIVE)
        {
            e_count += 1;
        }
        else if (cache[i].state == SHARED)
        {
            s_count += 1;
        }
        else if (cache[i].state == INVALID)
        {
            i_count += 1;
        }
    }

    /*
        M   E   S   I
    M   X   X   X   O
    E   X   X   X   O
    S   X   X   O   O
    I   O   O   O   O
    */

#ifdef DEBUG
    printf("M %d\t E %d\t S %d\t I %d\n", m_count, e_count, s_count, i_count);
#endif

    if ((m_count == 1 && i_count == (NUM_PROCESSOR - 1)) ||
        (e_count == 1 && i_count == (NUM_PROCESSOR - 1)) ||
        (s_count >= 2 && i_count == (NUM_PROCESSOR - s_count)) ||
        i_count == NUM_PROCESSOR
            ) {
        return 1;
    }

    return 0;
}


int read_line(int line, int *read_value) {

    if (cache[line].state == MODIFIED) {

        #ifdef DEBUG
        printf("[%d] read hit; dirty value %d\n", line, cache[line].value);
        #endif
        *read_value = cache[line].value;
        return 1;

    } else if (cache[line].state == EXCLUSIVE) {
        #ifdef DEBUG
        printf("[%d] read hit; exclusive clean value %d\n", line, cache[line].value);
        #endif
        *read_value = cache[line].value;
        return 1;
    } else if (cache[line].state == SHARED) {
        #ifdef DEBUG
        printf("[%d] read hit; shared clean value %d\n", line, cache[line].value);
        #endif
        *read_value = cache[line].value;
        return 1;
    }
    else if (cache[line].state == INVALID) {

        for (int i = 0; i < NUM_PROCESSOR; ++i) {

            if (i != line) {

                if (cache[i].state == MODIFIED) {

                    cache_value = cache[i].value;
                    cache[i].state = SHARED;

                    cache[line].state = SHARED;
                    cache[line].value = cache_value;

                    #ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies dirty value %d; write back; s_count == 2\n", line, i, cache[line].value);
                    #endif
                    *read_value = cache[line].value;

                    return 1;
                } else if (cache[i].state == EXCLUSIVE) {

                    cache[i].state = SHARED;

                    cache[line].state = SHARED;
                    cache[line].value = cache[i].value;
                    *read_value = cache[line].value;
                    #ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies clean value %d; s_count == 2\n", line, i, cache[line].value);
                    #endif
                    return 1;
                } else if (cache[i].state == SHARED) {

                    cache[line].state = SHARED;
                    cache[line].value = cache[i].value;
                    *read_value = cache[line].value;
                    #ifdef DEBUG
                    printf("[%d] read miss; [%d] supplies clean value %d; s_count >= 3\n", line , i, cache[line].value);
                    #endif
                    return 1;
                }

            }
        }

        cache[line].state = EXCLUSIVE;
        cache[line].value = cache_value;
        *read_value = cache[line].value;


        #ifdef DEBUG
        printf("[%d] read miss; mem supplies clean value %d; e_count == 1\n", line, cache[line].value);
        #endif
        return 1;
    }

    return 0;
}


int write_line(int line, int write_value) {

    if (cache[line].state == MODIFIED) {

        cache[line].value = write_value;
        cache[line].state = MODIFIED;

        #ifdef DEBUG
        printf("[%d] write hit; update to value %d\n", line, cache[line].value);
        #endif

        return 1;
    } else if (cache[line].state == EXCLUSIVE) {

        cache[line].value = write_value;
        cache[line].state = MODIFIED;

        #ifdef DEBUG
        printf("[%d] write hit; update to value %d\n", line, cache[line].value);
        #endif
        return 1;
    } else if (cache[line].state == SHARED) {

        cache[line].value = write_value;
        cache[line].state = MODIFIED;

        for (int i = 0; i < NUM_PROCESSOR; ++i) {
            if (line != i) {
                cache[i].value = 0;
                cache[i].state = INVALID;
            }
        }

        #ifdef DEBUG
        printf("[%d] write hit; update to value %d\n", line, cache[line].value);
        #endif
        return 1;

    } else if (cache[line].state == INVALID) {

        for (int i = 0; i < NUM_PROCESSOR; ++i) {

            if (i != line) {

                if (cache[i].state == MODIFIED) {

                    cache_value = cache[i].value;

                    cache[line].state = MODIFIED;
                    cache[line].value = write_value;

                    cache[i].state = INVALID;
                    cache[i].value = 0;

                    #ifdef DEBUG
                    printf("[%d] write miss; boardcast invalid to M; update to value %d\n", i, cache[i].value);
                    #endif
                    return 1;
                } else if (cache[i].state == EXCLUSIVE) {

                    cache[line].state = MODIFIED;
                    cache[line].value = write_value;
                    cache[i].state = INVALID;
                    cache[i].value = 0;

                    #ifdef DEBUG
                    printf("[%d] write miss; boardcast invalid to E; update to value %d\n", i, cache[i].value);
                    #endif
                    return 1;
                } else if (cache[i].state == SHARED) {

                    cache[line].state = MODIFIED;
                    cache[line].value = write_value;

                    for (int j = 0; j < NUM_PROCESSOR; ++j) {

                        if (j != line) {
                            cache[j].state = INVALID;
                            cache[j].value = 0;
                        }
                    }

                    #ifdef DEBUG
                    printf("[%d] write miss; boardcast invalid to S; update to value %d\n", i, cache[i].value);
                    #endif
                    return 1;
                }
            }
        }

        cache[line].state = MODIFIED;
        cache[line].value = write_value;
        #ifdef DEBUG
        printf("[%d] write miss; all invalid; update to value %d\n", line, cache[line].value);
        #endif
        return 1;
    }

    return 0;
}

int evict_line(int i)
{
    if (cache[i].state == MODIFIED)
    {
        // write back
        cache_value = cache[i].value;
        cache[i].state = INVALID;
        cache[i].value = 0;

#ifdef DEBUG
        printf("[%d] evict; write back value %d\n", i, cache[i].value);
#endif

        return 1;
    }
    else if (cache[i].state == EXCLUSIVE)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;

#ifdef DEBUG
        printf("[%d] evict\n", i);
#endif

        return 1;
    }
    else if (cache[i].state == SHARED)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;

        // may left only one shared to be exclusive
        int s_count = 0;
        int last_s = -1;

        for (int j = 0; j < NUM_PROCESSOR; ++ j)
        {
            if (cache[j].state == SHARED)
            {
                last_s = j;
                s_count ++;
            }
        }

        if (s_count == 1)
        {
            cache[last_s].state = EXCLUSIVE;
        }

#ifdef DEBUG
        printf("[%d] evict\n", i);
#endif

        return 1;
    }

    // evict when cache line is Invalid
    // not related with target physical address
    return 0;
}

void print_cacheline()
{
    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        char c;

        switch (cache[i].state)
        {
            case MODIFIED:
                c = 'M';
                break;
            case EXCLUSIVE:
                c = 'E';
                break;
            case SHARED:
                c = 'S';
                break;
            case INVALID:
                c = 'I';
                break;
            default:
                c = '?';
        }

        printf("\t[%d]      state %c        value %d\n", i, c, cache[i].value);
    }
        printf("\t                          mem value %d\n", cache_value);
}

int main()
{
    srand(123456);


    int read_value;

    for (int i = 0; i < NUM_PROCESSOR; ++ i)
    {
        cache[i].state = INVALID;
        cache[i].value = 0;
    }
    #ifdef DEBUG
    print_cacheline();
    #endif

    for (int i = 0; i < 100; ++ i)
    {
        int core_index = rand() % NUM_PROCESSOR;
        int op = rand() % 3;

    //    int do_print = 0;

        if (op == 0)
        {
 //            printf("read [%d]\n", core_index);
              read_line(core_index, &read_value);
//             printf("read line do_print %d\n", do_print);
        }
        else if (op == 1)
        {
 //            printf("write [%d]\n", core_index);
             write_line(core_index, rand() % 100);
//             printf("write line do_print %d\n", do_print);
        }
        else if (op == 2)
        {
  //            printf("evict [%d]\n", core_index);
              evict_line(core_index);
//             printf("evict line do_print %d\n", do_print);
        }

        #ifdef DEBUG
        if (do_print)
        {
            print_cacheline();
        }
        #endif

        if (check_states() == 0)
        {
            printf("failed\n");
            //print_cacheline();
            return 0;
        }
    }

    printf("pass\n");

    return 0;

}
