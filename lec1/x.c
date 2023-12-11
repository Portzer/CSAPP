//
// Created by M on 2023/12/3.
//
#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>

uint64_t add(uint64_t a, uint64_t b){
    uint64_t result = a + b;
    return result;
}
int main(){
    uint64_t a = 0x1234;
    uint64_t b = 0xabcd0000;
    uint64_t result = add(a, b);
    
}
