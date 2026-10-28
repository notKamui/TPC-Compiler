#ifndef ARM64_EMIT_H
#define ARM64_EMIT_H

/* Apple ARM64 assembly for the x86-64 instruction sequences translation.c emits. */

void arm_comment(const char *text);
void arm_label(const char *name);
void arm_push(const char *src);
void arm_pop(const char *dst);
void arm_mov(const char *dst, const char *src);
void arm_lea(const char *dst, const char *src);
void arm_add(const char *dst, const char *src);
void arm_sub(const char *dst, const char *src);
void arm_imul(const char *dst, const char *src);
void arm_idiv(const char *src);
void arm_neg(const char *reg);
void arm_cmp(const char *left, const char *right);
void arm_jump(const char *label);
void arm_jcc(const char *cond, const char *label);
void arm_setcc(const char *cond, const char *reg);
void arm_and(const char *dst, const char *src);
void arm_or(const char *dst, const char *src);
void arm_ret(void);
void arm_syscall(void);
void arm_call(const char *func);

void arm_data_header(void);
void arm_bss_header(void);
void arm_text_header(void);
void arm_reserve(const char *name, int bytes);
void arm_cstr(const char *name, const char *text);
void arm_stack_reserve(void);
void arm_main_enter(void);

#endif
