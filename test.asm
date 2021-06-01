section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss

section .text
extern printf
extern scanf
global main

abso:
	push rbp
	mov rbp, rsp
	; get value
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr literal
	push 0
	pop rax
	pop rbx
	cmp rax, rbx
	mov rax, 0
	setnc al
	push rax
	; instr if
	pop rax
	cmp rax, 0
	je ELSE0
	; get value
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr return 1st part
	pop rax
	; stack realign
	mov rsp, rbp
	pop rbp
	ret
	jmp ENDIF0
ELSE0:
	; get value
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr unary minus
	pop rax
	neg rax
	push rax
	; instr return 1st part
	pop rax
	; stack realign
	mov rsp, rbp
	pop rbp
	ret
ENDIF0:
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	pop rax
	; stack realign
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 4
	; instr literal
	push 0
	; instr assignment
	pop rax
	mov DWORD [rbp - (0)], eax
	; instr literal
	push 2
	; get value
	mov eax, DWORD [rbp - (0)]
	push rax
	pop rbx
	pop rax
	; instr int binop
	add rax, rbx
	push rax
	; instr assignment
	pop rax
	mov DWORD [rbp - (0)], eax
	; get value
	mov eax, DWORD [rbp - (0)]
	push rax
	; instr return 1st part
	pop rdi
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 2nd part
	mov rax, 60
	syscall
