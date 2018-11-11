.intel_syntax noprefix
.text

.global syscall1, syscall2, syscall3, syscall4, syscall5, _mmap

syscall:
    mov rax,rdi
    syscall
    ret

syscall1:
    mov rax,rdi
    mov rdi,rsi
    syscall
    ret

syscall2:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    syscall
    ret

syscall3:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx,rcx
    syscall
    ret

syscall4:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx,rcx
    mov r10,r8
    syscall
    ret

syscall5:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx,rcx
    mov r10,r8
    mov r8,r9
    syscall
    ret

_mmap:
	mov rax, 477
	mov rdi, 0
	mov rsi, 16384
	mov rdx, 7 # PROT_READ | PROT_WRITE | PROT_EXEC
	mov r10, 4096 # MAP_ANON
	mov r8, -1
	mov r9, 0
	syscall
	ret
