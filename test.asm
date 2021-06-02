section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss
	mdrt: resd 1

section .text
extern printf
extern scanf
global main

foo:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 8
	; instr literal
	push 4
	; instr assignment
	pop rax
	mov DWORD [rbp - (8)], eax
	; get lvalue
	mov al, BYTE [rbp - (-25)]
	push rax
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
	; instr literal
	push 122
	; instr return 1st part
	; add return value in stack
	mov rax, 1
STACK_RETURN0:
	cmp rax, 0
	je END_STACK_RETURN0
	mov r9, 1
	sub r9, rax
	mov bl, BYTE [rsp + r9]
	mov r9, 33
	sub r9, rax
	mov BYTE [rbp + r9], bl
	sub rax, 1
	jmp STACK_RETURN0
END_STACK_RETURN0:
	; stack realign
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 5
	sub rsp, 1
	mov r9, rsp
	; instr literal
	push 122
	; instr literal
	push 50
	; instr literal
	push 2
	pop rbx
	pop rax
	; instr int binop
	add rax, rbx
	push rax
	push r9
	call foo
	pop rsp
	; instr assignment
	pop rax
	mov BYTE [rbp - (5)], al
	; instr literal
	push 2
	; instr literal
	push 3
	pop rbx
	pop rax
	; instr int binop
	add rax, rbx
	push rax
	; instr assignment
	pop rax
	mov DWORD [rbp - (4)], eax
	; get lvalue
	mov al, BYTE [rbp - (3)]
	push rax
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
