#ifndef __TRANSLATION_UTILS__
#define __TRANSLATION_UTILS__

#include <stdio.h>

#include "abstract-tree.h"

#ifdef __APPLE__
#include "arm64_emit.h"
#define COMMENT(s) arm_comment(s)
#define LABEL(l) arm_label(l)
#define PUSH(r) arm_push(r)
#define POP(r) arm_pop(r)
#define MOV(r1, r2) arm_mov(r1, r2)
#define LEA(r1, r2) arm_lea(r1, r2)
#define ADD(r1, r2) arm_add(r1, r2)
#define SUB(r1, r2) arm_sub(r1, r2)
#define IMUL(r1, r2) arm_imul(r1, r2)
#define IDIV(r) arm_idiv(r)
#define NEG(r) arm_neg(r)
#define CMP(r1, r2) arm_cmp(r1, r2)
#define JMP(l) arm_jump(l)
#define JE(l) arm_jcc("eq", l)
#define JG(l) arm_jcc("gt", l)
#define SETZ(r) arm_setcc("eq", r)
#define SETNZ(r) arm_setcc("ne", r)
#define SETS(r) arm_setcc("mi", r)
#define SETG(r) arm_setcc("gt", r)
#define SETL(r) arm_setcc("lt", r)
#define SETLE(r) arm_setcc("le", r)
#define SETGE(r) arm_setcc("ge", r)
#define AND(r1, r2) arm_and(r1, r2)
#define OR(r1, r2) arm_or(r1, r2)
#define RET() arm_ret()
#define SYSCALL() arm_syscall()
#define CALL(f) arm_call(f)
#else
#include "elf64_emit.h"
#define COMMENT(s) elf_comment(s)
#define LABEL(l) elf_label(l)
#define PUSH(r) elf_push(r)
#define POP(r) elf_pop(r)
#define MOV(r1, r2) elf_mov(r1, r2)
#define LEA(r1, r2) elf_lea(r1, r2)
#define ADD(r1, r2) elf_add(r1, r2)
#define SUB(r1, r2) elf_sub(r1, r2)
#define IMUL(r1, r2) elf_imul(r1, r2)
#define IDIV(r) elf_idiv(r)
#define NEG(r) elf_neg(r)
#define CMP(r1, r2) elf_cmp(r1, r2)
#define JMP(l) elf_jump(l)
#define JE(l) elf_jcc("eq", l)
#define JG(l) elf_jcc("gt", l)
#define SETZ(r) elf_setcc("eq", r)
#define SETNZ(r) elf_setcc("ne", r)
#define SETS(r) elf_setcc("mi", r)
#define SETG(r) elf_setcc("gt", r)
#define SETL(r) elf_setcc("lt", r)
#define SETLE(r) elf_setcc("le", r)
#define SETGE(r) elf_setcc("ge", r)
#define AND(r1, r2) elf_and(r1, r2)
#define OR(r1, r2) elf_or(r1, r2)
#define RET() elf_ret()
#define SYSCALL() elf_syscall()
#define CALL(f) elf_call(f)
#endif

size_t prim_to_size(char *type);

char size_to_declsize(size_t size);

char *size_to_asmsize(size_t size);

void check_control_flow(const char *source, Node *func);

#endif