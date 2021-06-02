section .data
	fmtd: db "%d", 10, 0

section .bss
    init_stack: resq 1

section .text
extern printf

init_s: ; entree r15 = rbp
    mov QWORD [init_stack], r15 ; on stocke la valeur de rbp
    ret

disp_stack: ; entree: r15 = rsp
DISP_STACK_LOOP:
    cmp r15, [init_stack] ; test  de fin si rbp == rsp
	je END_DISP_STACK_LOOP
	
    mov r14, r15 ; registre tempo car on ne peut pas soustraire dans un acces
    neg r14
    mov sil, BYTE [rbp + r14]
	mov rax, 0
	mov rdi, fmtd
	call printf

	add r15, 8 ; incrementation d'un octet
	jmp DISP_STACK_LOOP
END_DISP_STACK_LOOP:
    ret