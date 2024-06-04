//
// Created by M on 2024/6/4.
//

#ifndef CSAPP_DATASTRUCT_H
#define CSAPP_DATASTRUCT_H
#include<stdint.h>

// extendible hash table
typedef struct
{
    int localdepth;     // the local depth
    int counter;        // the counter of slots (have data)
    char **karray;
    uint64_t *varray;
} bucket_t;

typedef struct
{
    int size;           // the size of (key, value) tuples of each bucket
    int num;            // number of buckets = 1 << globaldepth
    int globaldepth;    // the global depth

    bucket_t *barray;    // the internal table is actually an array
} hashtable_t;

hashtable_t *hashtable_construct(int bsize);
void hashtable_free(hashtable_t *tab);
int hashtable_get(hashtable_t *tab, char *key, uint64_t *val);
int hashtable_insert(hashtable_t **tab_addr, char *key, uint64_t val);
void print_hashtable(hashtable_t *tab);

#endif //CSAPP_DATASTRUCT_H
