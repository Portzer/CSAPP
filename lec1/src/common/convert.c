//
// Created by M on 2023/12/16.
//
/**
0 => 0
-0 => 0
0x0 => 0
1234 => 4d2
0x1234 => 1234
0xabcd => abcd
-0xabcd => ffffffffffff5433
-1234 => fffffffffffffb2e
2147483647 => 7fffffff
-2147483648 => ffffffff80000000
0x8000000000000000 => 8000000000000000
0xffffffffffffffff => ffffffffffffffff
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<header/common.h>
#define ONE2NINE(c)('1'<=(c)&&(c)<='9')
#define ZERO2NINE(c)('0'<=(c)&&(c)<='9')
#define A2F(c)('a'<=(c)&&(c)<='f')
#define BLANK(c)((c)==' ')
#define ZERO_M(c)((c)=='0')
#define NEG_M(c)((c)=='-')
#define X_M(c)((c)=='x'|| (c)=='X')

// covert string to int64_t
uint64_t string2uint(const char *str)
{
    return string2uint_range(str, 0, -1);
}
/**
 *定义状态
 *
 */
typedef enum NUM_STATUS{
    INIT,
    ZERO_NUM,
    DEC_NUM,
    NEG_NUM,
    X_NUM,
    HEX_NUM,
    BLANK_NUM

} num_status;
uint64_t string2uint_range(const char *str, int start, int end)
{
    end = (end == -1) ? strlen(str)-1  : end;
    uint64_t number = 0;
    //正负标志位
    int sign_ind = 0;

    int status = 0;

    for (int i = start; i <= end; ++i) {
        char c = str[i];
        if (status == INIT) {
            if (ZERO_M(c)) {
                status = ZERO_NUM;
                number = 0;
                continue;
            } else if (ONE2NINE(c)) {
                status = DEC_NUM;
                number = c - '0';
                continue;
            } else if (NEG_M(c)) {
                status = NEG_NUM;
                sign_ind = 1;
                continue;
            } else if (BLANK(c)) {
                status = INIT;
                continue;
            } else{
                goto fail;
            }
        } else if (status == ZERO_NUM) {
            if (X_M(c)) {
                status = X_NUM;
                continue;
            } else if (ZERO2NINE(c)) {
                status = DEC_NUM;
                number = number * 10 + c - '0';
                continue;
            } else if (BLANK(c)) {
                status = BLANK_NUM;
                continue;
            } else{
                goto fail;
            }
        } else if (status == DEC_NUM) {
            if (ZERO2NINE(c)) {
                status = DEC_NUM;
                uint64_t temp = number * 10 + c - '0';
                if (temp < number) {
                    printf("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
                number = temp;
                continue;
            } else {
                goto fail;
            }
        } else if (status == NEG_NUM) {
            if (ONE2NINE(c)) {
                status = DEC_NUM;
                number = c - '0';
                continue;
            } else if (ZERO_M(c)) {
                status = ZERO_NUM;
            } else {
                goto fail;
            }
        } else if (status == X_NUM) {
            if (ZERO2NINE(c)) {
                status = HEX_NUM;
                number = number * 16 + c - '0';
                continue;
            } else if (A2F(c)) {
                status = HEX_NUM;
                number = number * 16 + (c - 'a') + 10;
                continue;
            } else {
                goto fail;
            }
        } else if (status == HEX_NUM) {

            if (ZERO2NINE(c)) {
                status = HEX_NUM;
                uint64_t temp = number * 16 + (c - '0');
                if (temp < number) {
                    printf("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
                number = temp;
                continue;
            } else if (A2F(c)) {
                status = HEX_NUM;
                uint64_t temp = number * 16 + c - 'a' + 10;
                if (temp < number) {
                    printf("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
                number = temp;
                continue;
            } else {
                goto fail;
            }
        } else if (status == BLANK_NUM) {
            if (BLANK(c)) {
                status = BLANK_NUM;
                continue;
            } else {
                goto fail;
            }
        }
    }
    if (sign_ind == 0) {
        return number;
    } else if(sign_ind==1){
        if ((number & 0x8000000000000000) != 0)
        {
            printf("(int64_t)%s: signed overflow: cannot convert\n", str);
            exit(0);
        }
        int64_t sv = -1 * (int64_t) number;
        return *((uint64_t *)&sv);
    }

    fail:
    printf("type converter: <%s> cannot be converted to integer\n", str);
    exit(0);
}

// convert uint32_t to its float
uint32_t uint2float(uint32_t u)
{
    if (u == 0x00000000)
    {
        return 0x00000000;
    }
    // must be NORMALIZED
    // counting the position of highest 1: u[n]
    int n = 31;
    while (0 <= n && (((u >> n) & 0x1) == 0x0))
    {
        n = n - 1;
    }
    uint32_t e, f;
    //    seee eeee efff ffff ffff ffff ffff ffff
    // <= 0000 0000 1111 1111 1111 1111 1111 1111
    if (u <= 0x00ffffff)
    {
        // no need rounding
        uint32_t mask = 0xffffffff >> (32 - n);
        f = (u & mask) << (23 - n);
        e = n + 127;
        return (e << 23) | f;
    }
        // >= 0000 0001 0000 0000 0000 0000 0000 0000
    else
    {
        // need rounding
        // expand to 64 bit for situations like 0xffffffff
        uint64_t a = 0;
        a += u;
        // compute g, r, s
        uint32_t g = (a >> (n - 23)) & 0x1;     // Guard bit, the lowest bit of the result
        uint32_t r = (a >> (n - 24)) & 0x1;     // Round bit, the highest bit to be removed
        uint32_t s = 0x0;                       // Sticky bit, the OR of remaining bits in the removed part (low)
        for (int j = 0; j < n - 24; ++ j)
        {
            s = s | ((u >> j) & 0x1);
        }
        // compute carry
        a = a >> (n - 23);
        // 0    1    ?    ... ?
        // [24] [23] [22] ... [0]
        /* Rounding Rules
            +-------+-------+-------+-------+
            |   G   |   R   |   S   |       |
            +-------+-------+-------+-------+
            |   0   |   0   |   0   |   +0  | round down
            |   0   |   0   |   1   |   +0  | round down
            |   0   |   1   |   0   |   +0  | round down
            |   0   |   1   |   1   |   +1  | round up
            |   1   |   0   |   0   |   +0  | round down
            |   1   |   0   |   1   |   +0  | round down
            |   1   |   1   |   0   |   +1  | round up
            |   1   |   1   |   1   |   +1  | round up
            +-------+-------+-------+-------+
        carry = R & (G | S) by K-Map
        */
        if ((r & (g | s)) == 0x1)
        {
            a = a + 1;
        }
        // check carry
        if ((a >> 23) == 0x1)
        {
            // 0    1    ?    ... ?
            // [24] [23] [22] ... [0]
            f = a & 0x007fffff;
            e = n + 127;
            return (e << 23) | f;
        }
        else if ((a >> 23) == 0x2)
        {
            // 1    0    0    ... 0
            // [24] [23] [22] ... [0]
            e = n + 1 + 127;
            return (e << 23);
        }
    }
    // inf as default error
    return 0x7f800000;
}