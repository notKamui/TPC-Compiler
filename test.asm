section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss

section .text
extern printf
extern scanf
global main

main:
	pop rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 12
	; instr int literal
	push 5
	; instr assignment
	pop rax
	mov DWORD [rbp - 0], eax
	; instr char literal
	push 99
	; instr assignment
	pop rax
	mov BYTE [rbp - 8], al
	; get value
	mov al, BYTE [rbp - 8]
	push rax
	; instr return 1st part
	pop rdi
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 2nd part
	mov rax, 60
	syscall

