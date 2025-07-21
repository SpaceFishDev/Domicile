#ifndef SCHEDULER_H

#define SCHEDULER_H

#include <stdint.h>

typedef struct cpu_state
{
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
} cpu_state_t;

extern void save_cpu_state_impl(cpu_state_t *cpu_state);

#endif