section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss

section .text
extern printf
extern scanf
global main

main:
	push rbp
	mov rbp, rsp
	; instr literal
	push 5
	; instr literal
	push 6
	pop rbx
	pop rax
	cmp rax, rbx
	mov rax, 0
	setle al
	push rax
	; instr if
	pop rax
	cmp rax, 0
	je ENDIF0
	; instr literal
	push 1
	; instr return 1st part
	mov edi, DWORD [rsp]
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 2nd part
	mov rax, 60
	syscall
ENDIF0:
	; instr literal
	push 0
	; instr return 1st part
	mov edi, DWORD [rsp]
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 2nd part
	mov rax, 60
	syscall
