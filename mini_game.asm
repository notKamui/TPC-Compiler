section .data
	fmtd: db "%d", 0
	fmtc: db "%c", 0

section .bss
	n: resd 1

section .text
extern printf
extern scanf
global main

start_message:
	push rbp
	mov rbp, rsp
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 121
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 112
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 97
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 115
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 100
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 111
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 103
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 110
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 114
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 97
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 97
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 39
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 114
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 97
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 110
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 100
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 111
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 109
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 39
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 110
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 117
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 109
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 98
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 114
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
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
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

print_attempts:
	push rbp
	mov rbp, rsp
	; instr literal
	push 97
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 109
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 112
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtd
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 58
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
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

hint_more:
	push rbp
	mov rbp, rsp
	; instr literal
	push 109
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 111
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 114
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
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
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

hint_less:
	push rbp
	mov rbp, rsp
	; instr literal
	push 108
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 115
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 115
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
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

game:
	push rbp
	mov rbp, rsp
	; instr while
WHILE0:
	; push rvalue
	mov eax, DWORD [rbp - (-28)]
	push rax
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	mov rax, 0
	setnz al
	push rax
	pop rax
	cmp rax, 0
	je ENDWHILE0
	; instr assignment
	; push rvalue
	mov eax, DWORD [rbp - (-32)]
	push rax
	; instr literal
	push 1
	pop rbx
	pop rax
	; instr int binop
	add rax, rbx
	push rax
	pop rax
	mov DWORD [rbp - (-32)], eax
	mov r15, rsp
	; push rvalue
	mov eax, DWORD [rbp - (-32)]
	push rax
	mov eax, DWORD [rsp]
	mov DWORD [rsp + 4], eax
	add rsp, 4
	push r15
	call print_attempts
	pop rsp
	mov r9, rsp
	and spl, 240
	sub rsp, 8
	push r9
	mov rdi, fmtd
	lea rsi, [rbp - (-28)]
	mov rax, 0
	call scanf
	; push rvalue
	mov eax, DWORD [rbp - (-28)]
	push rax
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	pop rax
	pop rbx
	cmp rax, rbx
	mov rax, 0
	setl al
	push rax
	; instr if
	pop rax
	cmp rax, 0
	je ENDIF1
	mov r15, rsp
	push r15
	call hint_less
	pop rsp
ENDIF1:
	; push rvalue
	mov eax, DWORD [rbp - (-28)]
	push rax
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	pop rbx
	pop rax
	cmp rax, rbx
	mov rax, 0
	setl al
	push rax
	; instr if
	pop rax
	cmp rax, 0
	je ENDIF2
	mov r15, rsp
	push r15
	call hint_more
	pop rsp
ENDIF2:
	jmp WHILE0
ENDWHILE0:
	sub rsp, 12
	; struct assign
	mov rax, 0
STRUCT_ASSIGN3:
	cmp rax, 11
	jg END_STRUCT_ASSIGN3
	mov r9, rbp
	add r9, 11
	sub r9, -24
	sub r9, rax
	mov bl, BYTE [r9]
	mov r9, 11
	sub r9, rax
	mov BYTE [rsp + r9], bl
	add rax, 1
	jmp STRUCT_ASSIGN3
END_STRUCT_ASSIGN3:
	; instr return 1st part
	; add return value in stack
	mov rax, 0
STACK_RETURN4:
	cmp rax, 11
	jg END_STACK_RETURN4
	mov bl, BYTE [rsp + rax]
	mov r9, 36
	add r9, rax
	mov BYTE [rbp + r9], bl
	add rax, 1
	jmp STACK_RETURN4
END_STACK_RETURN4:
	; stack realign
	mov rsp, rbp
	pop rbp
	ret

end_message:
	push rbp
	mov rbp, rsp
	; instr literal
	push 119
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 112
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 121
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 111
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 117
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 102
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 111
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 117
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 110
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 100
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 104
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 110
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 117
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 109
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 98
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 114
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 105
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 110
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	mov eax, DWORD [n]
	push rax
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtd
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 97
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 101
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 109
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 112
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 116
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 40
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 115
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 41
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 32
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 33
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 33
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	; instr literal
	push 33
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
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

call_start_message:
	push rbp
	mov rbp, rsp
	mov r15, rsp
	push r15
	call start_message
	pop rsp
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

random_number:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 4
	; instr assignment
	mov DWORD [rbp - (4)], 1
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr literal
	push 2
	pop rbx
	pop rax
	; instr int binop
	mov rdx, 0
	idiv rbx
	push rdx
	; bool not
	pop rax
	cmp rax, 0
	mov rax, 0
	setz al
	push rax
	; instr literal
	push 1
	pop rbx
	pop rax
	; bool and
	cmp rax, 0
	mov rax, 0
	setnz al
	cmp rbx, 0
	mov rbx, 0
	setnz bl
	and rax, rbx
	mov rax, 0
	setnz al
	push rax
	; instr if
	pop rax
	cmp rax, 0
	je ELSE5
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr literal
	push 4
	pop rbx
	pop rax
	; instr int binop
	mov rdx, 0
	idiv rbx
	push rdx
	; instr literal
	push 0
	pop rbx
	pop rax
	cmp rax, rbx
	mov rax, 0
	setz al
	push rax
	; instr literal
	push 1
	; instr literal
	push 0
	pop rax
	pop rbx
	cmp rax, rbx
	mov rax, 0
	setle al
	push rax
	pop rbx
	pop rax
	; bool and
	cmp rax, 0
	mov rax, 0
	setnz al
	cmp rbx, 0
	mov rbx, 0
	setnz bl
	and rax, rbx
	mov rax, 0
	setnz al
	push rax
	; instr if
	pop rax
	cmp rax, 0
	je ENDIF6
	; instr assignment
	; instr literal
	push 1
	; instr unary minus
	pop rax
	neg rax
	push rax
	pop rax
	mov DWORD [rbp - (4)], eax
ENDIF6:
	jmp ENDIF5
ELSE5:
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr literal
	push 3
	pop rbx
	pop rax
	; instr int binop
	mov rdx, 0
	idiv rbx
	push rdx
	; instr literal
	push 0
	pop rbx
	pop rax
	cmp rax, rbx
	mov rax, 0
	setz al
	push rax
	; instr if
	pop rax
	cmp rax, 0
	je ENDIF7
	; instr assignment
	; instr literal
	push 1
	; instr unary minus
	pop rax
	neg rax
	push rax
	pop rax
	mov DWORD [rbp - (4)], eax
ENDIF7:
ENDIF5:
	; push rvalue
	mov eax, DWORD [rbp - (4)]
	push rax
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr literal
	push 53
	pop rbx
	pop rax
	; instr int binop
	add rax, rbx
	push rax
	; instr literal
	push 79
	pop rbx
	pop rax
	; instr int binop
	imul rax, rbx
	push rax
	; instr literal
	push 1
	; push rvalue
	mov eax, DWORD [rbp - (-24)]
	push rax
	; instr literal
	push 47
	pop rbx
	pop rax
	; instr int binop
	mov rdx, 0
	idiv rbx
	push rdx
	pop rbx
	pop rax
	; instr int binop
	add rax, rbx
	push rax
	pop rbx
	pop rax
	; instr int binop
	mov rdx, 0
	idiv rbx
	push rax
	pop rbx
	pop rax
	; instr int binop
	imul rax, rbx
	push rax
	; instr return 1st part
	; add return value in stack
	mov rax, 0
STACK_RETURN8:
	cmp rax, 3
	jg END_STACK_RETURN8
	mov bl, BYTE [rsp + rax]
	mov r9, 28
	add r9, rax
	mov BYTE [rbp + r9], bl
	add rax, 1
	jmp STACK_RETURN8
END_STACK_RETURN8:
	; stack realign
	mov rsp, rbp
	pop rbp
	ret

type_positive_number:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 4
	; instr assignment
	mov DWORD [rbp - (4)], 1
	; instr while
WHILE9:
	; push rvalue
	mov eax, DWORD [rbp - (4)]
	push rax
	; instr literal
	push 0
	pop rax
	pop rbx
	cmp rax, rbx
	mov rax, 0
	setl al
	push rax
	pop rax
	cmp rax, 0
	je ENDWHILE9
	mov r9, rsp
	and spl, 240
	sub rsp, 8
	push r9
	mov rdi, fmtd
	lea rsi, [rbp - (4)]
	mov rax, 0
	call scanf
	jmp WHILE9
ENDWHILE9:
	; instr assignment
	mov eax, DWORD [rbp - (4)]
	mov DWORD [n], eax
	; stack realign
	mov rsp, rbp
	pop rbp
	; instr return 1st part
	ret

main:
	push rbp
	mov rbp, rsp
	; local variables declaration
	sub rsp, 12
	; instr assignment
	; instr literal
	push 1
	; instr unary minus
	pop rax
	neg rax
	push rax
	pop rax
	mov DWORD [n], eax
	mov eax, DWORD [n]
	push rax
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtd
	call printf
	; instr literal
	push 10
	; instr print
	pop rsi
	mov rax, 0
	mov rdi, fmtc
	call printf
	mov r15, rsp
	push r15
	call call_start_message
	pop rsp
	; instr assignment
	sub rsp, 8
	mov r15, rsp
	mov eax, DWORD [n]
	push rax
	mov eax, DWORD [rsp]
	mov DWORD [rsp + 4], eax
	add rsp, 4
	push r15
	call random_number
	pop rsp
	pop rax
	mov DWORD [rbp - (12)], eax
	; instr assignment
	; push rvalue
	mov eax, DWORD [rbp - (12)]
	push rax
	; instr literal
	push 1
	pop rbx
	pop rax
	; instr int binop
	sub rax, rbx
	push rax
	pop rax
	mov DWORD [rbp - (8)], eax
	; instr assignment
	mov DWORD [rbp - (4)], 0
	; instr assignment
	sub rsp, 12
	mov r15, rsp
	sub rsp, 12
	; struct assign
	mov rax, 0
STRUCT_ASSIGN10:
	cmp rax, 11
	jg END_STRUCT_ASSIGN10
	mov r9, rbp
	sub r9, rax
	sub r9, 1
	sub r9, 0
	mov bl, BYTE [r9]
	mov r9, 11
	sub r9, rax
	mov BYTE [rsp + r9], bl
	add rax, 1
	jmp STRUCT_ASSIGN10
END_STRUCT_ASSIGN10:
	add rsp, 0
	push r15
	call game
	pop rsp
	; struct assign
	mov rax, 0
STRUCT_ASSIGN11:
	cmp rax, 11
	jg END_STRUCT_ASSIGN11
	mov r9, 11
	sub r9, rax
	mov bl, BYTE [rsp + r9]
	mov r9, rbp
	sub r9, rax
	sub r9, 1
	sub r9, 0
	mov BYTE [r9], bl
	add rax, 1
	jmp STRUCT_ASSIGN11
END_STRUCT_ASSIGN11:
	; instr assignment
	mov eax, DWORD [rbp - (4)]
	mov DWORD [n], eax
	mov r15, rsp
	push r15
	call end_message
	pop rsp
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
