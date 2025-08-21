#ifndef SYSCALL_H
#define SYSCALL_h

#include "../interrupts/interrupts.h"

enum syscall_table_values
{
    SYSCALL_READ,
    SYSCALL_WRITE,
    SYSCALL_OPEN,
    SYSCALL_FILE_SIZE,
    SYSCALL_CREATE_TEXTURE,
    SYSCALL_ADD_TEXTURE,
    SYSCALL_DELETE_TEXTURE,
    SYSCALL_RENDER_CLEAR,
    SYSCALL_RENDER_DRAW,
};

typedef struct
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax;
} user_registers_t;

void init_syscalls();

extern void syscall_handler();
extern void jump_usermode(uint64_t *user_stack_top);

#endif