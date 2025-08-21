global enter_userspace
section .text

enter_userspace:
    mov     cr3, rdi
    mov     rbx, rsi          ; rbx = tf pointer

    mov     r15, [rbx + 0x28]
    mov     r14, [rbx + 0x30]
    mov     r13, [rbx + 0x38]
    mov     r12, [rbx + 0x40]
    mov     r11, [rbx + 0x48]
    mov     r10, [rbx + 0x50]
    mov      r9, [rbx + 0x58]
    mov      r8, [rbx + 0x60]
    mov     rsi, [rbx + 0x68]
    mov     rdi, [rbx + 0x70]
    mov     rbp, [rbx + 0x78]
    mov     rdx, [rbx + 0x80]
    mov     rcx, [rbx + 0x88]
    mov     rax, [rbx + 0x98]
    mov     rbx, [rbx + 0x90]

    push    qword [rsi + 0x20]    ; SS
    push    qword [rsi + 0x18]    ; RSP
    push    qword [rsi + 0x10]    ; RFLAGS
    push    qword [rsi + 0x08]    ; CS
    push    qword [rsi + 0x00]    ; RIP

    iretq

global pit_irq_stub
extern pit_handler          

section .text
pit_irq_stub:
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov   rdi, rsp

    sub   rsp, 8
    call  pit_handler
    add   rsp, 8

    pop   r15
    pop   r14
    pop   r13
    pop   r12
    pop   r11
    pop   r10
    pop   r9
    pop   r8
    pop   rdi
    pop   rsi
    pop   rbp
    pop   rbx
    pop   rdx
    pop   rcx
    pop   rax

    iretq
