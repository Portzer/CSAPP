#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "../../header/cpu.h"
#include "../../header/memory.h"
#include "../../header/common.h"
#include "../../header/algorithm.h"
#include "../../header/instruction.h"


// functions to map the string assembly code to inst_t instance

static trie_node_t *register_mapping = NULL;
static trie_node_t *operator_mapping = NULL;

static void trie_cleanup()
{
    trie_free(register_mapping);
    trie_free(operator_mapping);
}

static void lazy_initialize_trie()
{
    // initialize the register mapping
    register_mapping = trie_construct();
    trie_insert(&register_mapping, "%rax",   (uint64_t)&(cpu_reg.rax)    );
    trie_insert(&register_mapping, "%eax",   (uint64_t)&(cpu_reg.eax)    );
    trie_insert(&register_mapping, "%ax",    (uint64_t)&(cpu_reg.ax)     );
    trie_insert(&register_mapping, "%ah",    (uint64_t)&(cpu_reg.ah)     );
    trie_insert(&register_mapping, "%al",    (uint64_t)&(cpu_reg.al)     );
    trie_insert(&register_mapping, "%rbx",   (uint64_t)&(cpu_reg.rbx)    );
    trie_insert(&register_mapping, "%ebx",   (uint64_t)&(cpu_reg.ebx)    );
    trie_insert(&register_mapping, "%bx",    (uint64_t)&(cpu_reg.bx)     );
    trie_insert(&register_mapping, "%bh",    (uint64_t)&(cpu_reg.bh)     );
    trie_insert(&register_mapping, "%bl",    (uint64_t)&(cpu_reg.bl)     );
    trie_insert(&register_mapping, "%rcx",   (uint64_t)&(cpu_reg.rcx)    );
    trie_insert(&register_mapping, "%ecx",   (uint64_t)&(cpu_reg.ecx)    );
    trie_insert(&register_mapping, "%cx",    (uint64_t)&(cpu_reg.cx)     );
    trie_insert(&register_mapping, "%ch",    (uint64_t)&(cpu_reg.ch)     );
    trie_insert(&register_mapping, "%cl",    (uint64_t)&(cpu_reg.cl)     );
    trie_insert(&register_mapping, "%rdx",   (uint64_t)&(cpu_reg.rdx)    );
    trie_insert(&register_mapping, "%edx",   (uint64_t)&(cpu_reg.edx)    );
    trie_insert(&register_mapping, "%dx",    (uint64_t)&(cpu_reg.dx)     );
    trie_insert(&register_mapping, "%dh",    (uint64_t)&(cpu_reg.dh)     );
    trie_insert(&register_mapping, "%dl",    (uint64_t)&(cpu_reg.dl)     );
    trie_insert(&register_mapping, "%rsi",   (uint64_t)&(cpu_reg.rsi)    );
    trie_insert(&register_mapping, "%esi",   (uint64_t)&(cpu_reg.esi)    );
    trie_insert(&register_mapping, "%si",    (uint64_t)&(cpu_reg.si)     );
    trie_insert(&register_mapping, "%sih",   (uint64_t)&(cpu_reg.sih)    );
    trie_insert(&register_mapping, "%sil",   (uint64_t)&(cpu_reg.sil)    );
    trie_insert(&register_mapping, "%rdi",   (uint64_t)&(cpu_reg.rdi)    );
    trie_insert(&register_mapping, "%edi",   (uint64_t)&(cpu_reg.edi)    );
    trie_insert(&register_mapping, "%di",    (uint64_t)&(cpu_reg.di)     );
    trie_insert(&register_mapping, "%dih",   (uint64_t)&(cpu_reg.dih)    );
    trie_insert(&register_mapping, "%dil",   (uint64_t)&(cpu_reg.dil)    );
    trie_insert(&register_mapping, "%rbp",   (uint64_t)&(cpu_reg.rbp)    );
    trie_insert(&register_mapping, "%ebp",   (uint64_t)&(cpu_reg.ebp)    );
    trie_insert(&register_mapping, "%bp",    (uint64_t)&(cpu_reg.bp)     );
    trie_insert(&register_mapping, "%bph",   (uint64_t)&(cpu_reg.bph)    );
    trie_insert(&register_mapping, "%bpl",   (uint64_t)&(cpu_reg.bpl)    );
    trie_insert(&register_mapping, "%rsp",   (uint64_t)&(cpu_reg.rsp)    );
    trie_insert(&register_mapping, "%esp",   (uint64_t)&(cpu_reg.esp)    );
    trie_insert(&register_mapping, "%sp",    (uint64_t)&(cpu_reg.sp)     );
    trie_insert(&register_mapping, "%sph",   (uint64_t)&(cpu_reg.sph)    );
    trie_insert(&register_mapping, "%spl",   (uint64_t)&(cpu_reg.spl)    );
    trie_insert(&register_mapping, "%r8",    (uint64_t)&(cpu_reg.r8)     );
    trie_insert(&register_mapping, "%r8d",   (uint64_t)&(cpu_reg.r8d)    );
    trie_insert(&register_mapping, "%r8w",   (uint64_t)&(cpu_reg.r8w)    );
    trie_insert(&register_mapping, "%r8b",   (uint64_t)&(cpu_reg.r8b)    );
    trie_insert(&register_mapping, "%r9",    (uint64_t)&(cpu_reg.r9)     );
    trie_insert(&register_mapping, "%r9d",   (uint64_t)&(cpu_reg.r9d)    );
    trie_insert(&register_mapping, "%r9w",   (uint64_t)&(cpu_reg.r9w)    );
    trie_insert(&register_mapping, "%r9b",   (uint64_t)&(cpu_reg.r9b)    );
    trie_insert(&register_mapping, "%r10",   (uint64_t)&(cpu_reg.r10)    );
    trie_insert(&register_mapping, "%r10d",  (uint64_t)&(cpu_reg.r10d)   );
    trie_insert(&register_mapping, "%r10w",  (uint64_t)&(cpu_reg.r10w)   );
    trie_insert(&register_mapping, "%r10b",  (uint64_t)&(cpu_reg.r10b)   );
    trie_insert(&register_mapping, "%r11",   (uint64_t)&(cpu_reg.r11)    );
    trie_insert(&register_mapping, "%r11d",  (uint64_t)&(cpu_reg.r11d)   );
    trie_insert(&register_mapping, "%r11w",  (uint64_t)&(cpu_reg.r11w)   );
    trie_insert(&register_mapping, "%r11b",  (uint64_t)&(cpu_reg.r11b)   );
    trie_insert(&register_mapping, "%r12",   (uint64_t)&(cpu_reg.r12)    );
    trie_insert(&register_mapping, "%r12d",  (uint64_t)&(cpu_reg.r12d)   );
    trie_insert(&register_mapping, "%r12w",  (uint64_t)&(cpu_reg.r12w)   );
    trie_insert(&register_mapping, "%r12b",  (uint64_t)&(cpu_reg.r12b)   );
    trie_insert(&register_mapping, "%r13",   (uint64_t)&(cpu_reg.r13)    );
    trie_insert(&register_mapping, "%r13d",  (uint64_t)&(cpu_reg.r13d)   );
    trie_insert(&register_mapping, "%r13w",  (uint64_t)&(cpu_reg.r13w)   );
    trie_insert(&register_mapping, "%r13b",  (uint64_t)&(cpu_reg.r13b)   );
    trie_insert(&register_mapping, "%r14",   (uint64_t)&(cpu_reg.r14)    );
    trie_insert(&register_mapping, "%r14d",  (uint64_t)&(cpu_reg.r14d)   );
    trie_insert(&register_mapping, "%r14w",  (uint64_t)&(cpu_reg.r14w)   );
    trie_insert(&register_mapping, "%r14b",  (uint64_t)&(cpu_reg.r14b)   );
    trie_insert(&register_mapping, "%r15",   (uint64_t)&(cpu_reg.r15)    );
    trie_insert(&register_mapping, "%r15d",  (uint64_t)&(cpu_reg.r15d)   );
    trie_insert(&register_mapping, "%r15w",  (uint64_t)&(cpu_reg.r15w)   );
    trie_insert(&register_mapping, "%r15b",  (uint64_t)&(cpu_reg.r15b)   );

    // initialize the operator mapping
    operator_mapping = trie_construct();
    trie_insert(&operator_mapping, "movq",   INST_MOV    );
    trie_insert(&operator_mapping, "mov",    INST_MOV    );
    trie_insert(&operator_mapping, "push",   INST_PUSH   );
    trie_insert(&operator_mapping, "pop",    INST_POP    );
    trie_insert(&operator_mapping, "leaveq", INST_LEAVE  );
    trie_insert(&operator_mapping, "callq",  INST_CALL   );
    trie_insert(&operator_mapping, "retq",   INST_RET    );
    trie_insert(&operator_mapping, "add",    INST_ADD    );
    trie_insert(&operator_mapping, "sub",    INST_SUB    );
    trie_insert(&operator_mapping, "cmpq",   INST_CMP    );
    trie_insert(&operator_mapping, "jne",    INST_JNE    );
    trie_insert(&operator_mapping, "jmp",    INST_JMP    );

    trie_print(operator_mapping);
    trie_print(register_mapping);

    // add the cleanup events
    add_cleanup_event(&trie_cleanup);
}


static uint64_t try_get_from_trie(trie_node_t **root, char *key)
{
    if (*root == NULL)
    {
        lazy_initialize_trie();
    }
    uint64_t val;
    int result = trie_get(*root, key, &val);
    if (result == 0)
    {
        printf("could not find key '%s' from trie\n", key);
        exit(0);
    }
    return val;
}

static void parse_instruction(const char *str, inst_t *inst, core_t *cr);
static void parse_operand(const char *str, od_t *od, core_t *cr);
static uint64_t decode_operand(od_t *od);



// interpret the operand
static uint64_t decode_operand(od_t *od)
{
    if (od->type == IMM)
    {
        // immediate signed number can be negative: convert to bitmap
        return *(uint64_t *)&od->imm;
    }
    else if (od->type == REG)
    {
        // default register 1
        return od->reg1;
    }
    else if (od->type == EMPTY)
    {
        return 0;
    }
    else
    {
        // access memory: return the physical address
        uint64_t vaddr = 0;

        if (od->type == MEM_IMM)
        {
            vaddr = od->imm;
        }
        else if (od->type == MEM_REG1)
        {
            vaddr = *((uint64_t *)od->reg1);
        }
        else if (od->type == MEM_IMM_REG1)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg1));
        }
        else if (od->type == MEM_REG1_REG2)
        {
            vaddr = (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2));
        }
        else if (od->type == MEM_IMM_REG1_REG2)
        {
            vaddr = od->imm +  (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2));
        }
        else if (od->type == MEM_REG2_SCAL)
        {
            vaddr = (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == MEM_IMM_REG2_SCAL)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == MEM_REG1_REG2_SCAL)
        {
            vaddr = (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == MEM_IMM_REG1_REG2_SCAL)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2)) * od->scal;
        }
        return vaddr;
    }

    // empty
    return 0;
}

static void parse_instruction(const char *str, inst_t *inst, core_t *cr)
{
    char op_str[64] = {'\0'};
    int op_len = 0;
    char src_str[64] = {'\0'};
    int src_len = 0;
    char dst_str[64] = {'\0'};
    int dst_len = 0;
    int bc = 0;
    int status = 0;
    for (int i = 0; i < strlen(str); ++i) {
        char c = str[i];
        if (c == '(' || c == ')') {
            bc++;
        }
        if (status == 0 && c != ' ') {
            status = 1;
        }
        if (status == 1 && c == ' ') {
            status = 2;
            continue;
        }
        if (status == 2 && c != ' ') {
            status = 3;
        }
        if (status == 3 && c == ',') {
            status = 4;
            continue;
        }
        if (status == 4 && c != ' ' && (bc == 0 || bc == 2)) {
            status = 5;
        }
        if (status == 5 && c == ' ') {
            status = 6;
            continue;
        }
        if (status == 1) {
            op_str[op_len] = c;
            op_len++;
            continue;
        }
        if (status == 3) {
            src_str[src_len] = c;
            src_len++;
            continue;
        }
        if (status == 5) {
            dst_str[dst_len] = c;
            dst_len++;
            continue;
        }
    }
    parse_operand(src_str, &(inst->src), cr);
    parse_operand(dst_str, &(inst->dst), cr);

    inst->op = (op_t)try_get_from_trie(&operator_mapping, op_str);


    debug_printf(DEBUG_PARSEINST, "[%s (%d)] [%s (%d)] [%s (%d)]\n", op_str, inst->op, src_str, inst->src.type, dst_str, inst->dst.type);
}

static void parse_operand(const char *str, od_t *od, core_t *cr)
{
    od->type = EMPTY;
    od->imm = 0;
    od->reg1 = 0;
    od->reg2 = 0;
    od->scal = 0;
    if (strlen(str) == 0) {
        return;
    }
    //立即数
    if (str[0] == '$') {
        od->type = IMM;
        od->imm = string2uint_range(str, 1, -1);
        return;
        //寄存器
    } else if (str[0]=='%') {
        od->type = REG;
        od->reg1 = try_get_from_trie(&register_mapping, (char *)str);
        return;
    } else {
        int imm_len = 0;
        char imm[64] = {'\0'};
        int reg1_len = 0;
        char reg1[64] = {'\0'};
        int reg2_len = 0;
        char reg2[64] = {'\0'};
        int scal_len = 0;
        char scal[64] = {'\0'};
        int len = strlen(str);
        // ()数量
        int bc = 0;
        //,数量
        int cc = 0;
        for (int i = 0; i < len; ++i) {
            char c = str[i];
            //统计符号数量
            if (c == '(' || c == ')') {
                bc++;
                continue;
            } else if (c == ',') {
                cc++;
                continue;
            }
            //统计立即数，寄存器1，寄存器2，scal的数量
            if (bc == 0) {
                imm[imm_len] = c;
                imm_len++;
                continue;
            } else if (bc == 1) {
                //xxxx(xxxxx
                //(xxxx
                if (cc == 0) {
                    reg1[reg1_len] = c;
                    reg1_len++;
                    continue;
                    //xxxx(xxxxx,xxxx
                    //(xxxxxx,xxxxxx
                    //(,xxxxxxx
                } else if (cc == 1) {
                    reg2[reg2_len] = c;
                    reg2_len++;
                    continue;
                    //xxxx(xxxxx,xxxxx,x
                    //(xxxx,xxxx,x
                    //(,xxxxx,x
                } else if (cc == 2) {
                    scal[scal_len] = c;
                    scal_len++;
                    continue;
                }
            }
        }
        if (imm_len > 0) {
            od->imm = string2uint(imm);
        }
        if (reg1_len > 0) {
            printf("reg1 %s \n", reg1);
            od->reg1 = try_get_from_trie(&register_mapping, (char *)str);

        }
        if (reg2_len > 0) {
            od->reg1 = try_get_from_trie(&register_mapping, (char *)str);
        }
        if (scal_len > 0) {
            uint64_t scal_t = string2uint(scal);
            if (scal_t != 1 && scal_t != 2 && scal_t != 4 && scal_t != 8) {
                printf("scal is error %lx \n", scal_t);
                exit(1);
            }
            od->scal = scal_t;
        }
        //一个()都没有
        if (bc == 0) {
            od->type = MEM_IMM;
            return;
        }
        //,一个没有xxx(xxx) or (xxxx)
        if (cc == 0) {
            //是否有立即数
            if (imm_len > 0) {
                od->type = MEM_IMM_REG1;
                return;
            } else {
                od->type = MEM_REG1;
                return;
            }
            //只有一个
        } else if (cc == 1) {
            if (imm_len > 0) {
                od->type = MEM_IMM_REG1_REG2;
                return;
            } else {
                od->type = MEM_REG1_REG2;
                return;
            }
        } else if (cc == 2) {
            if (imm_len > 0) {
                if (reg1_len > 0) {
                    od->type = MEM_IMM_REG1_REG2_SCAL;
                    return;
                } else {
                    od->type = MEM_IMM_REG2_SCAL;
                    return;
                }
            } else {
                if (reg1_len > 0) {
                    od->type = MEM_REG1_REG2_SCAL;
                    return;
                } else {
                    od->type = MEM_REG2_SCAL;
                    return;
                }
            }
        }
    }
}

/*======================================*/
/*      instruction handlers            */
/*======================================*/

// insturction (sub)set
// In this simulator, the instructions have been decoded and fetched
// so there will be no page fault during fetching
// otherwise the instructions must handle the page fault (swap in from disk) first
// and then re-fetch the instruction and do decoding
// and finally re-run the instruction

static void mov_handler             (od_t *src_od, od_t *dst_od, core_t *cr);
static void push_handler            (od_t *src_od, od_t *dst_od, core_t *cr);
static void pop_handler             (od_t *src_od, od_t *dst_od, core_t *cr);
static void leave_handler           (od_t *src_od, od_t *dst_od, core_t *cr);
static void call_handler            (od_t *src_od, od_t *dst_od, core_t *cr);
static void ret_handler             (od_t *src_od, od_t *dst_od, core_t *cr);
static void add_handler             (od_t *src_od, od_t *dst_od, core_t *cr);
static void sub_handler             (od_t *src_od, od_t *dst_od, core_t *cr);
static void cmp_handler             (od_t *src_od, od_t *dst_od, core_t *cr);
static void jne_handler             (od_t *src_od, od_t *dst_od, core_t *cr);
static void jmp_handler             (od_t *src_od, od_t *dst_od, core_t *cr);

// handler table storing the handlers to different instruction types
typedef void (*handler_t)(od_t *, od_t *, core_t *);
// look-up table of pointers to function
static handler_t handler_table[NUM_INSTRTYPE] = {
        &mov_handler,               // 0
        &push_handler,              // 1
        &pop_handler,               // 2
        &leave_handler,             // 3
        &call_handler,              // 4
        &ret_handler,               // 5
        &add_handler,               // 6
        &sub_handler,               // 7
        &cmp_handler,               // 8
        &jne_handler,               // 9
        &jmp_handler,               // 10
};

// reset the condition flags
// inline to reduce cost
static inline void reset_cflags(core_t *cr)
{
    cr->flags.OF = 0;
    cr->flags.SF = 0;
    cr->flags.ZF = 0;
    cr->flags.CF = 0;
}

// update the rip pointer to the next instruction sequentially
static inline void next_rip(core_t *cr)
{
    // we are handling the fixed-length of assembly string here
    // but their size can be variable as true X86 instructions
    // that's because the operands' sizes follow the specific encoding rule
    // the risc-v is a fixed length ISA
    cr->rip = cr->rip + sizeof(char) * MAX_INSTRUCTION_CHAR;
}

// instruction handlers

static void mov_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
    uint64_t src = decode_operand(src_od);
    uint64_t dst = decode_operand(dst_od);

    if (src_od->type == REG && dst_od->type == REG)
    {
        // src: register
        // dst: register
        *(uint64_t *)dst = *(uint64_t *)src;
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
    else if (src_od->type == REG && dst_od->type >= MEM_IMM)
    {
        // src: register
        // dst: virtual address
        write64bits_dram(
                va2pa(dst, cr),
                *(uint64_t *)src,
                cr
        );
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
    else if (src_od->type >= MEM_IMM && dst_od->type == REG)
    {
        // src: virtual address
        // dst: register
        *(uint64_t *)dst = read64bits_dram(
                va2pa(src, cr),
                cr);
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
    else if (src_od->type == IMM && dst_od->type == REG)
    {
        // src: immediate number (uint64_t bit map)
        // dst: register
        *(uint64_t *)dst = src;
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
}

static void push_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
    uint64_t src = decode_operand(src_od);
    // uint64_t dst = decode_operand(dst_od);

    if (src_od->type == REG)
    {
        // src: register
        // dst: empty
        (cr->reg).rsp = (cr->reg).rsp - 8;
        write64bits_dram(
                va2pa((cr->reg).rsp, cr),
                *(uint64_t *)src,
                cr
        );
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
}

static void pop_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
    uint64_t src = decode_operand(src_od);
    // uint64_t dst = decode_operand(dst_od);

    if (src_od->type == REG)
    {
        // src: register
        // dst: empty
        uint64_t old_val = read64bits_dram(
                va2pa((cr->reg).rsp, cr),
                cr
        );
        (cr->reg).rsp = (cr->reg).rsp + 8;
        *(uint64_t *)src = old_val;
        next_rip(cr);
        reset_cflags(cr);
        return;
    }
}

static void leave_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
}

static void call_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
    uint64_t src = decode_operand(src_od);
    // uint64_t dst = decode_operand(dst_od);

    // src: immediate number: virtual address of target function starting
    // dst: empty
    // push the return value
    (cr->reg).rsp = (cr->reg).rsp - 8;
    write64bits_dram(
            va2pa((cr->reg).rsp, cr),
            cr->rip + sizeof(char) * MAX_INSTRUCTION_CHAR,
            cr
    );
    // jump to target function address
    cr->rip = src;
    reset_cflags(cr);
}

static void ret_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
    // uint64_t src = decode_operand(src_od);
    // uint64_t dst = decode_operand(dst_od);

    // src: empty
    // dst: empty
    // pop rsp
    uint64_t ret_addr = read64bits_dram(
            va2pa((cr->reg).rsp, cr),
            cr
    );
    (cr->reg).rsp = (cr->reg).rsp + 8;
    // jump to return address
    cr->rip = ret_addr;
    reset_cflags(cr);
}

static void add_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
    uint64_t src = decode_operand(src_od);
    uint64_t dst = decode_operand(dst_od);

    if (src_od->type == REG && dst_od->type == REG)
    {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        uint64_t val = *(uint64_t *)dst + *(uint64_t *)src;
        int src_ind = (src >> 63 & 0x1);
        int dst_ind = (src >> 63 & 0x1);
        int val_ind = (val >> 63 & 0x1);
        // set condition flags
        cr->flags.ZF = (val == 0);
        cr->flags.CF = (val < src);
        cr->flags.SF = (val >> 63 & 0x1);
        cr->flags.OF = (((val_ind == 0) && (src_ind == 1 && dst_ind == 1)) ||
                        ((val_ind == 1) && (src_ind == 0 && dst_ind == 0)));


        // update registers
        *(uint64_t *)dst = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        next_rip(cr);
        return;
    }
}

static void sub_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
    // src: register (value: int64_t bit map)
    // dst: register (value: int64_t bit map)
    // dst = dst - src = dst + (-src)
    uint64_t src = decode_operand(src_od);
    uint64_t dst = decode_operand(dst_od);
    uint64_t val = *(uint64_t *)dst + (~src + 1);

    int val_sign = ((val >> 63) & 0x1);
    int src_sign = ((src >> 63) & 0x1);
    int dst_sign = ((*(uint64_t *)dst >> 63) & 0x1);

    // set condition flags
    cr->flags.CF = (val > *(uint64_t *)dst); // unsigned

    cr->flags.ZF = (val == 0);
    cr->flags.SF = val_sign;

    cr->flags.OF = (src_sign == 1 && dst_sign == 0 && val_sign == 1) || (src_sign == 0 && dst_sign == 1 && val_sign == 0);

    // update registers
    *(uint64_t *)dst = val;
    // signed and unsigned value follow the same addition. e.g.
    // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
    next_rip(cr);
    return;
}

static void cmp_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
}

static void jne_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
}

static void jmp_handler(od_t *src_od, od_t *dst_od, core_t *cr)
{
}

// instruction cycle is implemented in CPU
// the only exposed interface outside CPU
void instruction_cycle(core_t *cr)
{
    // FETCH: get the instruction string by program counter
    char inst_str[MAX_INSTRUCTION_CHAR+10];
    readinst_dram(va2pa(cr->rip, cr), inst_str, cr);
    debug_printf(DEBUG_INSTRUCTIONCYCLE, "%lx    %s\n", cr->rip, inst_str);

    // DECODE: decode the run-time instruction operands
    inst_t inst;
    parse_instruction(inst_str, &inst, cr);

    printf("cycle opt is %d \n", inst.op);
    // EXECUTE: get the function pointer or handler by the operator
    handler_t handler = handler_table[inst.op];
    // update CPU and memory according the instruction
    handler(&(inst.src), &(inst.dst), cr);
}

void print_register(core_t *cr)
{
    if ((DEBUG_VERBOSE_SET & DEBUG_REGISTERS) == 0x0)
    {
        return;
    }

    reg_t reg = cr->reg;

    printf("rax = %lx\trbx = %lx\trcx = %lx\trdx = %lx\n",
           reg.rax, reg.rbx, reg.rcx, reg.rdx);
    printf("rsi = %lx\trdi = %lx\trbp = %lx\trsp = %lx\n",
           reg.rsi, reg.rdi, reg.rbp, reg.rsp);
    printf("rip = %lx\n", cr->rip);
    printf("CF = %u\tZF = %u\tSF = %u\tOF = %u\n",
           cr->flags.CF, cr->flags.ZF, cr->flags.SF, cr->flags.OF);
}

void print_stack(core_t *cr)
{
    if ((DEBUG_VERBOSE_SET & DEBUG_PRINTSTACK) == 0x0)
    {
        return;
    }

    int n = 10;
    uint64_t *high = (uint64_t*)&pm[va2pa((cr->reg).rsp, cr)];
    high = &high[n];
    uint64_t va = (cr->reg).rsp + n * 8;

    for (int i = 0; i < 2 * n; ++ i)
    {
        uint64_t *ptr = (uint64_t *)(high - i);
        printf("0x%lx : %lx", va, (uint64_t)*ptr);

        if (i == n)
        {
            printf(" <== rsp");
        }
        printf("\n");
        va -= 8;
    }
}
void TestParsingOperand()
{
    ACTIVE_CORE = 0x0;
    core_t *ac = (core_t *)&CORES[ACTIVE_CORE];

    const char *strs[11] = {
            "$0x1234",
            "%rax",
            "0xabcd",
            "(%rsp)",
            "0xabcd(%rsp)",
            "(%rsp,%rbx)",
            "0xabcd(%rsp,%rbx)",
            "(,%rbx,8)",
            "0xabcd(,%rbx,8)",
            "(%rsp,%rbx,8)",
            "0xabcd(%rsp,%rbx,8)",
    };

    printf("rax %p\n", &(ac->reg.rax));
    printf("rsp %p\n", &(ac->reg.rsp));
    printf("rbx %p\n", &(ac->reg.rbx));

    for (int i = 0; i < 11; ++ i)
    {
        od_t od;
        parse_operand(strs[i], &od, ac);

        printf("\n%s\n", strs[i]);
        printf("od enum type: %d\n", od.type);
        printf("od imm: %lx\n", od.imm);
        printf("od reg1:%lx\n", od.reg1);
        printf("od reg2:%lx\n", od.reg2);
        printf("od scal: %lx\n", od.scal);
    }
}