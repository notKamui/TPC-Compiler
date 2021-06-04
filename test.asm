section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss

section .text
extern printf
extern scanf
global main

bar:
	push rbp
	mov rbp, rsp
	sub rsp, 4
	; struct assign
	mov rax, 0
STRUCT_ASSIGN0:
	cmp rax, 3
	jg END_STRUCT_ASSIGN0
	mov r9, rbp
	add r9, 3
	sub r9, -24
	sub r9, rax
	mov bl, BYTE [r9]
	mov r9, 3
	sub r9, rax
	mov BYTE [rsp + r9], bl
	add rax, 1
	jmp STRUCT_ASSIGN0
END_STRUCT_ASSIGN0:
	; instr return 1st part
	; add return value in stack
	mov rax, 0
STACK_RETURN1:
	cmp rax, 3
	jg END_STACK_RETURN1
	mov bl, BYTE [rsp + rax]
	mov r9, 28
	add r9, rax
	mov BYTE [rbp + r9], bl
	add rax, 1
	jmp STACK_RETURN1
END_STACK_RETURN1:
	; stack realign
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 10
	; instr while
WHILE2:
	; push rvalue
	mov eax, DWORD [rbp - (6)]
	push rax
	; push rvalue
	mov al, BYTE [rbp - (1)]
	push rax
	pop rax
	pop rbx
	cmp rax, rbx
	mov rax, 0
	setl al
	push rax
	pop rax
	cmp rax, 0
	je ENDWHILE2
	; instr literal
	push 97
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 10
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	jmp WHILE2
ENDWHILE2:
	; push rvalue
	mov eax, DWORD [rbp - (6)]
	push rax
	; instr return 1st part
	mov edi, DWORD [rsp]
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 2nd part
	mov rax, 60
	syscall
