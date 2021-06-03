section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss
	xd: resb 8
	mdr: resb 8

section .text
extern printf
extern scanf
global main

main:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 4
	; instr assignment
	mov DWORD [rbp - (4)], 5
	; push rvalue
	mov eax, DWORD [rbp - (4)]
	push rax
	; instr return 1st part
	mov edi, DWORD [rsp]
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 2nd part
	mov rax, 60
	syscall
