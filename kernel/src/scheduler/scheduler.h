#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../memory/pagetables.h"
#include <stdint.h>

typedef struct trapframe
{

    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} trapframe_t;

typedef enum
{
    PROC_READY,
    PROC_RUNNING,
    PROC_KILLED,
} proc_state_t;

typedef struct process
{
    uint64_t pid;
    proc_state_t state;
    char name[11];
    page_table_manager_t page_table_manager;
    uint64_t cr3;
    void *kernel_stack_top;
    void *user_stack_top;
    void *entry;
    trapframe_t *tf;
    struct process *next;
    int exit_code;
} proccess_t;

extern proccess_t *base_proc;

void create_process(proccess_t *proc, char *name, void *entry);
void scheduler_tick(trapframe_t *tf);
void start_proc(proccess_t *proc);
extern proccess_t *base_proc;
extern proccess_t *current_proc;

extern void enter_userspace(uint64_t cr3, trapframe_t *tf);
extern void pit_irq_stub();

#endif