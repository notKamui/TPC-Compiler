#ifndef __TRANSLATION_UTILS__
#define __TRANSLATION_UTILS__

#include <stdio.h>

#define COMMENT(s) fprintf(file, "\t; %s\n", s)
#define LABEL(l) fprintf(file, "%s:\n", l)
#define PUSH(r) fprintf(file, "\tpush %s\n", r)
#define POP(r) fprintf(file, "\tpop %s\n", r)
#define MOV(r1, r2) fprintf(file, "\tmov %s, %s\n", r1, r2)
#define LEA(r1, r2) fprintf(file, "\tlea %s, %s\n", r1, r2)
#define ADD(r1, r2) fprintf(file, "\tadd %s, %s\n", r1, r2)
#define SUB(r1, r2) fprintf(file, "\tsub %s, %s\n", r1, r2)
#define IMUL(r1, r2) fprintf(file, "\timul %s, %s\n", r1, r2)
#define IDIV(r) fprintf(file, "\tidiv %s\n", r)
#define NEG(r) fprintf(file, "\tneg %s\n", r)
#define CMP(r1, r2) fprintf(file, "\tcmp %s, %s\n", r1, r2)
#define JMP(l) fprintf(file, "\tjmp %s\n", l)
#define JE(l) fprintf(file, "\tje %s\n", l)
#define SETZ(r) fprintf(file, "\tsetz %s\n", r)
#define SETNZ(r) fprintf(file, "\tsetnz %s\n", r)
#define SETS(r) fprintf(file, "\tsets %s\n", r)
#define SETG(r) fprintf(file, "\tsetg %s\n", r)
#define SETL(r) fprintf(file, "\tsetl %s\n", r)
#define SETLE(r) fprintf(file, "\tsetle %s\n", r)
#define SETGE(r) fprintf(file, "\tsetge %s\n", r)
#define AND(r1, r2) fprintf(file, "\tand %s, %s\n", r1, r2)
#define OR(r1, r2) fprintf(file, "\tor %s, %s\n", r1, r2)
#define RET() fprintf(file, "\tret\n")
#define SYSCALL() fprintf(file, "\tsyscall\n")
#define CALL(f) fprintf(file, "\tcall %s\n", f)

size_t prim_to_size(char *type);

char size_to_declsize(size_t size);

char *size_to_asmsize(size_t size);

#endif