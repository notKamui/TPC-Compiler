#include "elf64_emit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The operand strings from translation.c are already NASM operands. */

extern FILE *file;

static void die(const char *msg, const char *detail) {
    fprintf(stderr, "elf64 backend: %s", msg);
    if (detail != NULL) {
        fprintf(stderr, ": %s", detail);
    }
    fprintf(stderr, "\n");
    exit(1);
}

static const char *jcc_insn(const char *cond) {
    if (strcmp(cond, "eq") == 0) {
        return "je";
    }
    if (strcmp(cond, "gt") == 0) {
        return "jg";
    }
    die("unsupported conditional jump", cond);
    return NULL;
}

static const char *setcc_insn(const char *cond) {
    if (strcmp(cond, "eq") == 0) {
        return "setz";
    }
    if (strcmp(cond, "ne") == 0) {
        return "setnz";
    }
    if (strcmp(cond, "mi") == 0) {
        return "sets";
    }
    if (strcmp(cond, "gt") == 0) {
        return "setg";
    }
    if (strcmp(cond, "lt") == 0) {
        return "setl";
    }
    if (strcmp(cond, "le") == 0) {
        return "setle";
    }
    if (strcmp(cond, "ge") == 0) {
        return "setge";
    }
    die("unsupported conditional set", cond);
    return NULL;
}

void elf_comment(const char *text) {
    fputs("\t; ", file);
    fputs(text, file);
    fputc('\n', file);
}

void elf_label(const char *name) {
    fprintf(file, "%s:\n", name);
}

void elf_push(const char *src) {
    fprintf(file, "\tpush %s\n", src);
}

void elf_pop(const char *dst) {
    fprintf(file, "\tpop %s\n", dst);
}

void elf_mov(const char *dst, const char *src) {
    fprintf(file, "\tmov %s, %s\n", dst, src);
}

void elf_lea(const char *dst, const char *src) {
    fprintf(file, "\tlea %s, %s\n", dst, src);
}

void elf_add(const char *dst, const char *src) {
    fprintf(file, "\tadd %s, %s\n", dst, src);
}

void elf_sub(const char *dst, const char *src) {
    fprintf(file, "\tsub %s, %s\n", dst, src);
}

void elf_imul(const char *dst, const char *src) {
    fprintf(file, "\timul %s, %s\n", dst, src);
}

void elf_idiv(const char *src) {
    fprintf(file, "\tidiv %s\n", src);
}

void elf_neg(const char *reg) {
    fprintf(file, "\tneg %s\n", reg);
}

void elf_cmp(const char *left, const char *right) {
    fprintf(file, "\tcmp %s, %s\n", left, right);
}

void elf_jump(const char *label) {
    fprintf(file, "\tjmp %s\n", label);
}

void elf_jcc(const char *cond, const char *label) {
    fprintf(file, "\t%s %s\n", jcc_insn(cond), label);
}

void elf_setcc(const char *cond, const char *reg) {
    fprintf(file, "\t%s %s\n", setcc_insn(cond), reg);
}

void elf_and(const char *dst, const char *src) {
    fprintf(file, "\tand %s, %s\n", dst, src);
}

void elf_or(const char *dst, const char *src) {
    fprintf(file, "\tor %s, %s\n", dst, src);
}

void elf_ret(void) {
    fputs("\tret\n", file);
}

void elf_syscall(void) {
    fputs("\tsyscall\n", file);
}

void elf_call(const char *func) {
    fprintf(file, "\tcall %s\n", func);
}

void elf_data_header(void) {
    fputs("; ELF64 NASM assembly\n", file);
    fputs("section .data\n", file);
}

void elf_bss_header(void) {
    fputs("section .bss\n", file);
}

void elf_text_header(void) {
    fputs("section .text\n", file);
    fputs("extern printf\n", file);
    fputs("extern scanf\n", file);
    fputs("global main\n\n", file);
}

void elf_reserve(const char *name, int bytes) {
    if (bytes == 4) {
        fprintf(file, "\t%s: resd 1\n", name);
    } else if (bytes == 1) {
        fprintf(file, "\t%s: resb 1\n", name);
    } else {
        fprintf(file, "\t%s: resb %d\n", name, bytes);
    }
}

void elf_cstr(const char *name, const char *text) {
    fprintf(file, "\t%s: db \"", name);
    fputs(text, file);
    fputs("\", 0\n", file);
}

void elf_stack_reserve(void) {
    /* ELF64 uses the process stack, not a reserved buffer. */
}

void elf_main_enter(void) {
    /* The C runtime enters main with a usable stack. */
}
