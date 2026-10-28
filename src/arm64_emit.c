#include "arm64_emit.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Virtual stack lives in x19 so it can move by 8 bytes. Real sp stays 16-byte
 * aligned for printf and scanf. x29 is the TPC frame pointer. */

extern FILE *file;

static int ret_id;

typedef struct {
    const char *x;
    const char *w;
    int bits;
} Reg;

typedef struct {
    enum { OP_REG, OP_IMM, OP_MEM, OP_SYM } kind;
    Reg reg;
    int64_t imm;
    int mem_bits;
    int base_is_sym;
    char sym[96];
    const char *base_x;
    const char *index_x;
    int64_t disp;
} Operand;

static void die(const char *msg, const char *a, const char *b) {
    fprintf(stderr, "arm64 backend: %s", msg);
    if (a != NULL) {
        fprintf(stderr, ": %s", a);
    }
    if (b != NULL) {
        fprintf(stderr, ", %s", b);
    }
    fprintf(stderr, "\n");
    exit(1);
}

static void skip(const char **p) {
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

static int lookup_reg(const char *name, Reg *reg) {
    static const struct {
        const char *name;
        const char *x;
        const char *w;
        int bits;
    } table[] = {
        {"rax", "x9", "w9", 64},  {"eax", "x9", "w9", 32},  {"al", "x9", "w9", 8},
        {"rbx", "x10", "w10", 64}, {"ebx", "x10", "w10", 32}, {"bl", "x10", "w10", 8},
        {"rdx", "x11", "w11", 64}, {"edx", "x11", "w11", 32},
        {"rdi", "x0", "w0", 64},  {"edi", "x0", "w0", 32},
        {"rsi", "x1", "w1", 64},  {"esi", "x1", "w1", 32},
        {"r9", "x12", "w12", 64},
        {"r15", "x13", "w13", 64},
        {"rbp", "x29", "w29", 64},
        {"rsp", "x19", "w19", 64},
        {"spl", "x19", "w19", 8},
    };
    size_t i;

    for (i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (strcmp(name, table[i].name) == 0) {
            reg->x = table[i].x;
            reg->w = table[i].w;
            reg->bits = table[i].bits;
            return 1;
        }
    }
    return 0;
}

static int parse_number(const char **p, int64_t *out) {
    char *end;
    long long value;

    value = strtoll(*p, &end, 10);
    if (end == *p) {
        return 0;
    }
    *p = end;
    *out = (int64_t)value;
    return 1;
}

static int parse_ident(const char **p, char *out, size_t outlen) {
    size_t n = 0;

    if (!isalpha((unsigned char)**p) && **p != '_') {
        return 0;
    }
    while (isalnum((unsigned char)**p) || **p == '_') {
        if (n + 1 >= outlen) {
            die("identifier too long", *p, NULL);
        }
        out[n++] = **p;
        (*p)++;
    }
    out[n] = '\0';
    return 1;
}

static int parse_mem(const char **p, Operand *op) {
    char ident[96];
    Reg base;

    skip(p);
    if (**p != '[') {
        return 0;
    }
    (*p)++;
    skip(p);
    if (!parse_ident(p, ident, sizeof(ident))) {
        return 0;
    }
    if (lookup_reg(ident, &base)) {
        op->base_is_sym = 0;
        op->base_x = base.x;
        op->sym[0] = '\0';
    } else {
        op->base_is_sym = 1;
        op->base_x = NULL;
        snprintf(op->sym, sizeof(op->sym), "%s", ident);
    }
    op->index_x = NULL;
    op->disp = 0;
    skip(p);
    if (**p == '+' || **p == '-') {
        int minus = **p == '-';
        (*p)++;
        skip(p);
        if (minus) {
            int64_t inner;
            if (**p != '(') {
                return 0;
            }
            (*p)++;
            skip(p);
            if (!parse_number(p, &inner)) {
                return 0;
            }
            skip(p);
            if (**p != ')') {
                return 0;
            }
            (*p)++;
            op->disp = -inner;
        } else if (isdigit((unsigned char)**p) || **p == '-') {
            if (!parse_number(p, &op->disp)) {
                return 0;
            }
        } else {
            Reg index;
            if (!parse_ident(p, ident, sizeof(ident)) || !lookup_reg(ident, &index)) {
                return 0;
            }
            op->index_x = index.x;
        }
    }
    skip(p);
    if (**p != ']') {
        return 0;
    }
    (*p)++;
    skip(p);
    return **p == '\0';
}

static int parse_operand(const char *text, Operand *op) {
    const char *p = text;
    char ident[96];
    char *end;
    long long imm;

    memset(op, 0, sizeof(*op));
    skip(&p);
    if (strncmp(p, "BYTE", 4) == 0 && (p[4] == ' ' || p[4] == '\t' || p[4] == '[')) {
        op->mem_bits = 8;
        p += 4;
        skip(&p);
    } else if (strncmp(p, "DWORD", 5) == 0 && (p[5] == ' ' || p[5] == '\t' || p[5] == '[')) {
        op->mem_bits = 32;
        p += 5;
        skip(&p);
    }
    if (*p == '[') {
        op->kind = OP_MEM;
        if (op->mem_bits == 0) {
            op->mem_bits = 64;
        }
        return parse_mem(&p, op);
    }
    imm = strtoll(p, &end, 10);
    if (end != p && *end == '\0') {
        op->kind = OP_IMM;
        op->imm = (int64_t)imm;
        return 1;
    }
    if (!parse_ident(&p, ident, sizeof(ident)) || *p != '\0') {
        return 0;
    }
    if (lookup_reg(ident, &op->reg)) {
        op->kind = OP_REG;
        return 1;
    }
    op->kind = OP_SYM;
    snprintf(op->sym, sizeof(op->sym), "%s", ident);
    return 1;
}

static Operand must_parse(const char *text, const char *what) {
    Operand op;
    if (!parse_operand(text, &op)) {
        die(what, text, NULL);
    }
    return op;
}

static void emit_mov_imm(const char *reg, uint64_t value) {
    int wide = reg[0] == 'x';
    int shift;

    fprintf(file, "\tmovz %s, #%llu\n", reg, (unsigned long long)(value & 0xffffu));
    for (shift = 16; shift < (wide ? 64 : 32); shift += 16) {
        unsigned long long part = (value >> shift) & 0xffffu;
        if (part != 0) {
            fprintf(file, "\tmovk %s, #%llu, lsl #%d\n", reg, part, shift);
        }
    }
}

static void emit_add_imm(const char *dst, const char *src, int64_t delta) {
    const char *op = "add";

    if (delta < 0) {
        op = "sub";
        delta = -delta;
    }
    if (delta == 0) {
        if (strcmp(dst, src) != 0) {
            fprintf(file, "\tmov %s, %s\n", dst, src);
        }
        return;
    }
    if (delta <= 4095) {
        fprintf(file, "\t%s %s, %s, #%lld\n", op, dst, src, (long long)delta);
        return;
    }
    emit_mov_imm("x17", (uint64_t)delta);
    fprintf(file, "\t%s %s, %s, x17\n", op, dst, src);
}

static void load_addr(const Operand *mem) {
    if (mem->base_is_sym) {
        fprintf(file, "\tadrp x16, _%s@PAGE\n", mem->sym);
        fprintf(file, "\tadd x16, x16, _%s@PAGEOFF\n", mem->sym);
    } else if (mem->index_x == NULL && mem->disp == 0) {
        fprintf(file, "\tmov x16, %s\n", mem->base_x);
        return;
    } else {
        emit_add_imm("x16", mem->base_x, mem->disp);
    }
    if (mem->disp != 0 && mem->base_is_sym) {
        emit_add_imm("x16", "x16", mem->disp);
    }
    if (mem->index_x != NULL) {
        fprintf(file, "\tadd x16, x16, %s\n", mem->index_x);
    }
}

static void load_mem(const Reg *dest, const Operand *mem) {
    const char *width;

    load_addr(mem);
    if (mem->mem_bits == 8) {
        width = dest->w;
        fprintf(file, "\tldrb %s, [x16]\n", width);
    } else if (mem->mem_bits == 32) {
        fprintf(file, "\tldr %s, [x16]\n", dest->w);
    } else {
        fprintf(file, "\tldr %s, [x16]\n", dest->x);
    }
}

static void store_reg(const Operand *mem, const Reg *src) {
    load_addr(mem);
    if (mem->mem_bits == 8) {
        fprintf(file, "\tstrb %s, [x16]\n", src->w);
    } else if (mem->mem_bits == 32) {
        fprintf(file, "\tstr %s, [x16]\n", src->w);
    } else {
        fprintf(file, "\tstr %s, [x16]\n", src->x);
    }
}

static void note2(const char *mnem, const char *a, const char *b) {
    fputs("\t// ", file);
    fputs(mnem, file);
    if (a != NULL) {
        fputs(" ", file);
        fputs(a, file);
    }
    if (b != NULL) {
        fputs(", ", file);
        fputs(b, file);
    }
    fputc('\n', file);
}

void arm_comment(const char *text) {
    fputs("\t// ", file);
    fputs(text, file);
    fputc('\n', file);
}

void arm_label(const char *name) {
    fprintf(file, ".p2align 2\n_%s:\n", name);
}

void arm_push(const char *src) {
    Operand op;

    note2("push", src, NULL);
    op = must_parse(src, "bad push");
    fputs("\tsub x19, x19, #8\n", file);
    if (op.kind == OP_IMM) {
        emit_mov_imm("x17", (uint64_t)op.imm);
        fputs("\tstr x17, [x19]\n", file);
    } else if (op.kind == OP_REG) {
        fprintf(file, "\tstr %s, [x19]\n", op.reg.x);
    } else {
        die("bad push", src, NULL);
    }
}

void arm_pop(const char *dst) {
    Operand op;

    note2("pop", dst, NULL);
    op = must_parse(dst, "bad pop");
    if (op.kind != OP_REG) {
        die("bad pop", dst, NULL);
    }
    if (strcmp(dst, "rsp") == 0) {
        /* pop rsp loads the new stack pointer and does not then add 8. */
        fputs("\tldr x19, [x19]\n", file);
        return;
    }
    fprintf(file, "\tldr %s, [x19]\n", op.reg.x);
    fputs("\tadd x19, x19, #8\n", file);
}

void arm_mov(const char *dst, const char *src) {
    Operand d, s;

    note2("mov", dst, src);
    d = must_parse(dst, "bad mov destination");
    s = must_parse(src, "bad mov source");
    if (d.kind == OP_REG && s.kind == OP_REG) {
        if (d.reg.bits == 8) {
            fprintf(file, "\tand %s, %s, #0xffffffffffffff00\n", d.reg.x, d.reg.x);
            fprintf(file, "\tand x17, %s, #0xff\n", s.reg.x);
            fprintf(file, "\torr %s, %s, x17\n", d.reg.x, d.reg.x);
        } else if (d.reg.bits == 32) {
            fprintf(file, "\tmov %s, %s\n", d.reg.w, s.reg.w);
        } else {
            fprintf(file, "\tmov %s, %s\n", d.reg.x, s.reg.x);
        }
    } else if (d.kind == OP_REG && s.kind == OP_IMM) {
        if (d.reg.bits <= 32) {
            emit_mov_imm(d.reg.w, (uint64_t)s.imm);
        } else {
            emit_mov_imm(d.reg.x, (uint64_t)s.imm);
        }
    } else if (d.kind == OP_REG && s.kind == OP_SYM) {
        fprintf(file, "\tadrp %s, _%s@PAGE\n", d.reg.x, s.sym);
        fprintf(file, "\tadd %s, %s, _%s@PAGEOFF\n", d.reg.x, d.reg.x, s.sym);
    } else if (d.kind == OP_REG && s.kind == OP_MEM) {
        load_mem(&d.reg, &s);
    } else if (d.kind == OP_MEM && s.kind == OP_REG) {
        store_reg(&d, &s.reg);
    } else if (d.kind == OP_MEM && s.kind == OP_IMM) {
        load_addr(&d);
        if (d.mem_bits == 8) {
            emit_mov_imm("w17", (uint64_t)s.imm & 0xffu);
            fputs("\tstrb w17, [x16]\n", file);
        } else {
            emit_mov_imm("w17", (uint64_t)s.imm & 0xffffffffu);
            fputs("\tstr w17, [x16]\n", file);
        }
    } else {
        die("unsupported mov", dst, src);
    }
}

void arm_lea(const char *dst, const char *src) {
    Operand d, s;

    note2("lea", dst, src);
    d = must_parse(dst, "bad lea destination");
    s = must_parse(src, "bad lea source");
    if (d.kind != OP_REG || s.kind != OP_MEM) {
        die("unsupported lea", dst, src);
    }
    load_addr(&s);
    fprintf(file, "\tmov %s, x16\n", d.reg.x);
}

static void arith(const char *op, const char *dst, const char *src, int set_flags) {
    Operand d, s;
    const char *mnem = op;

    d = must_parse(dst, "bad arithmetic destination");
    s = must_parse(src, "bad arithmetic source");
    if (d.kind != OP_REG) {
        die("bad arithmetic destination", dst, src);
    }
    if (set_flags && strcmp(op, "and") == 0) {
        mnem = "ands";
    } else if (set_flags && strcmp(op, "orr") == 0) {
        mnem = "orrs";
    }
    if (s.kind == OP_IMM) {
        if (strcmp(op, "add") == 0) {
            emit_add_imm(d.reg.x, d.reg.x, s.imm);
        } else if (strcmp(op, "sub") == 0) {
            emit_add_imm(d.reg.x, d.reg.x, -s.imm);
        } else {
            die("immediate arithmetic is only supported for add and sub", dst, src);
        }
        return;
    }
    if (s.kind != OP_REG) {
        die("bad arithmetic source", dst, src);
    }
    fprintf(file, "\t%s %s, %s, %s\n", mnem, d.reg.x, d.reg.x, s.reg.x);
}

void arm_add(const char *dst, const char *src) {
    note2("add", dst, src);
    arith("add", dst, src, 0);
}

void arm_sub(const char *dst, const char *src) {
    note2("sub", dst, src);
    arith("sub", dst, src, 0);
}

void arm_and(const char *dst, const char *src) {
    Operand imm;

    note2("and", dst, src);
    if (strcmp(dst, "spl") == 0) {
        imm = must_parse(src, "bad spl mask");
        if (imm.kind != OP_IMM) {
            die("spl mask must be immediate", dst, src);
        }
        /* and spl, mask keeps bits 8+ and applies mask to the low byte. */
        fputs("\tand x17, x19, #0xff\n", file);
        fprintf(file, "\tand x17, x17, #%lld\n", (long long)imm.imm);
        fputs("\tand x19, x19, #0xffffffffffffff00\n", file);
        fputs("\torr x19, x19, x17\n", file);
        return;
    }
    arith("and", dst, src, 1);
}

void arm_or(const char *dst, const char *src) {
    note2("or", dst, src);
    arith("orr", dst, src, 1);
}

void arm_imul(const char *dst, const char *src) {
    Operand d, s;

    note2("imul", dst, src);
    d = must_parse(dst, "bad imul destination");
    s = must_parse(src, "bad imul source");
    if (d.kind != OP_REG || s.kind != OP_REG) {
        die("unsupported imul", dst, src);
    }
    fprintf(file, "\tmul %s, %s, %s\n", d.reg.x, d.reg.x, s.reg.x);
}

void arm_idiv(const char *src) {
    Operand s;

    note2("idiv", src, NULL);
    s = must_parse(src, "bad idiv");
    if (s.kind != OP_REG) {
        die("bad idiv", src, NULL);
    }
    /* rdx is cleared before idiv, so the dividend is the value in rax. */
    fprintf(file, "\tsdiv x17, x9, %s\n", s.reg.x);
    fprintf(file, "\tmsub x11, x17, %s, x9\n", s.reg.x);
    fputs("\tmov x9, x17\n", file);
}

void arm_neg(const char *reg) {
    Operand r;

    note2("neg", reg, NULL);
    r = must_parse(reg, "bad neg");
    if (r.kind != OP_REG) {
        die("bad neg", reg, NULL);
    }
    fprintf(file, "\tneg %s, %s\n", r.reg.x, r.reg.x);
}

void arm_cmp(const char *left, const char *right) {
    Operand l, r;

    note2("cmp", left, right);
    l = must_parse(left, "bad cmp left");
    r = must_parse(right, "bad cmp right");
    if (l.kind != OP_REG) {
        die("bad cmp left", left, right);
    }
    if (r.kind == OP_REG) {
        if (l.reg.bits <= 32 && r.reg.bits <= 32) {
            fprintf(file, "\tcmp %s, %s\n", l.reg.w, r.reg.w);
        } else {
            fprintf(file, "\tcmp %s, %s\n", l.reg.x, r.reg.x);
        }
    } else if (r.kind == OP_IMM) {
        if (r.imm >= 0 && r.imm <= 4095) {
            fprintf(file, "\tcmp %s, #%lld\n", l.reg.x, (long long)r.imm);
        } else {
            emit_mov_imm("x17", (uint64_t)r.imm);
            fprintf(file, "\tcmp %s, x17\n", l.reg.x);
        }
    } else {
        die("bad cmp right", left, right);
    }
}

void arm_jump(const char *label) {
    note2("jmp", label, NULL);
    fprintf(file, "\tb _%s\n", label);
}

void arm_jcc(const char *cond, const char *label) {
    note2(strcmp(cond, "eq") == 0 ? "je" : "jg", label, NULL);
    fprintf(file, "\tb.%s _%s\n", cond, label);
}

void arm_setcc(const char *cond, const char *reg) {
    Operand r;

    note2("setcc", cond, reg);
    r = must_parse(reg, "bad setcc");
    if (r.kind != OP_REG) {
        die("bad setcc", reg, NULL);
    }
    fprintf(file, "\tcset %s, %s\n", r.reg.w, cond);
}

void arm_ret(void) {
    fputs("\t// ret\n", file);
    fputs("\tldr x16, [x19]\n", file);
    fputs("\tadd x19, x19, #8\n", file);
    fputs("\tbr x16\n", file);
}

void arm_syscall(void) {
    /* Replaces Linux exit (syscall 60). edi already holds the status in w0. */
    fputs("\t// syscall exit\n", file);
    fputs("\tldr x19, [sp, #16]\n", file);
    fputs("\tldp x29, x30, [sp], #32\n", file);
    fputs("\tret\n", file);
}

void arm_call(const char *func) {
    note2("call", func, NULL);
    if (strcmp(func, "printf") == 0 || strcmp(func, "scanf") == 0) {
        /* Darwin passes variadic arguments on the stack. x0 is the format
         * and x1 is the value or pointer. */
        fputs("\tsub sp, sp, #16\n", file);
        fputs("\tstr x1, [sp]\n", file);
        fprintf(file, "\tbl _%s\n", func);
        fputs("\tadd sp, sp, #16\n", file);
        return;
    }
    fprintf(file, "\tsub x19, x19, #8\n");
    fprintf(file, "\tadrp x16, Ltpc_ret_%d@PAGE\n", ret_id);
    fprintf(file, "\tadd x16, x16, Ltpc_ret_%d@PAGEOFF\n", ret_id);
    fputs("\tstr x16, [x19]\n", file);
    fprintf(file, "\tb _%s\n", func);
    fprintf(file, "Ltpc_ret_%d:\n", ret_id);
    ret_id++;
}

void arm_data_header(void) {
    fputs("// ARM64 Mach-O assembly\n", file);
    fputs(".section __DATA,__data\n", file);
}

void arm_bss_header(void) {
    fputs(".section __DATA,__bss\n", file);
}

void arm_text_header(void) {
    fputs(".section __TEXT,__text,regular,pure_instructions\n", file);
    fputs(".globl _main\n", file);
}

void arm_reserve(const char *name, int bytes) {
    if (bytes >= 4) {
        fputs("\t.p2align 2\n", file);
    }
    fprintf(file, "_%s:\n\t.space %d\n", name, bytes);
}

void arm_cstr(const char *name, const char *text) {
    fprintf(file, "_%s:\n\t.asciz \"", name);
    fputs(text, file);
    fputs("\"\n", file);
}

void arm_stack_reserve(void) {
    fputs("\t.p2align 4\n", file);
    fputs("_tpc_stack_area:\n", file);
    fputs("\t.space 8388608\n", file);
    fputs("_tpc_stack_top:\n", file);
}

void arm_main_enter(void) {
    fputs("\tstp x29, x30, [sp, #-32]!\n", file);
    fputs("\tstr x19, [sp, #16]\n", file);
    fputs("\tadrp x19, _tpc_stack_top@PAGE\n", file);
    fputs("\tadd x19, x19, _tpc_stack_top@PAGEOFF\n", file);
}
