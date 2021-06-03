section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss
	test: resb 8

section .text
extern printf
extern scanf
global main

coucou:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 8
	; instr assignment
	mov DWORD [rbp - (4)], 100
	; instr assignment
	; struct assign
	mov rax, 0
STRUCT_ASSIGN0:
	cmp rax, 7
	jg END_STRUCT_ASSIGN0
	mov r9, rbp
	sub r9, rax
	sub r9, 1
	sub r9, 0
	mov bl, BYTE [r9]
	mov r9, 7
	sub r9, rax
	mov BYTE [test + r9], bl
	add rax, 1
	jmp STRUCT_ASSIGN0
END_STRUCT_ASSIGN0:
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

main:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 16
	; instr assignment
	mov DWORD [rbp - (12)], 50
	; instr assignment
	mov DWORD [rbp - (16)], 5
	; instr assignment
	; struct assign
	mov rax, 0
STRUCT_ASSIGN1:
	cmp rax, 7
	jg END_STRUCT_ASSIGN1
	mov r9, rbp
	sub r9, rax
	sub r9, 1
	sub r9, 8
	mov bl, BYTE [r9]
	mov r9, rbp
	sub r9, rax
	sub r9, 1
	sub r9, 0
	mov BYTE [r9], bl
	add rax, 1
	jmp STRUCT_ASSIGN1
END_STRUCT_ASSIGN1:
	; instr assignment
	; push rvalue
	mov eax, DWORD [rbp - (4)]
	push rax
	; instr literal
	push 2
	pop rbx
	pop rax
	; instr int binop
	add rax, rbx
	push rax
	pop rax
	mov DWORD [rbp - (4)], eax
	mov r9, rsp
	push r9
	call coucou
	pop rsp
	mov eax, DWORD [test + 4]
	push rax
	; instr return 1st part
	mov edi, DWORD [rsp]
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 2nd part
	mov rax, 60
	syscall
