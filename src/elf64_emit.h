#ifndef ELF64_EMIT_H
#define ELF64_EMIT_H

/* ELF64 NASM assembly for the x86-64 instruction sequences translation.c emits. */

void elf_comment(const char *text);
void elf_label(const char *name);
void elf_push(const char *src);
void elf_pop(const char *dst);
void elf_mov(const char *dst, const char *src);
void elf_lea(const char *dst, const char *src);
void elf_add(const char *dst, const char *src);
void elf_sub(const char *dst, const char *src);
void elf_imul(const char *dst, const char *src);
void elf_idiv(const char *src);
void elf_neg(const char *reg);
void elf_cmp(const char *left, const char *right);
void elf_jump(const char *label);
void elf_jcc(const char *cond, const char *label);
void elf_setcc(const char *cond, const char *reg);
void elf_and(const char *dst, const char *src);
void elf_or(const char *dst, const char *src);
void elf_ret(void);
void elf_syscall(void);
void elf_call(const char *func);

void elf_data_header(void);
void elf_bss_header(void);
void elf_text_header(void);
void elf_reserve(const char *name, int bytes);
void elf_cstr(const char *name, const char *text);
void elf_stack_reserve(void);
void elf_main_enter(void);

#endif
