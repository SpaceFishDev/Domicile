#include "scheduler.h"
#include "../memory/malloc.h"
#include "../utils/string.h"

#define KCODE_SEL 0x08
#define KDATA_SEL 0x10
#define UCODE_SEL (0x20 | 0x3)
#define UDATA_SEL (0x28 | 0x3)

proccess_t *base_proc = 0;
proccess_t *current_proc = 0;

void create_process(proccess_t *proc, char *name, void *entry)
{
    memset(proc, 0, sizeof(*proc));
    int i = 0;
    for (i = 0; i < 10 && name[i]; ++i)
    {
        proc->name[i] = name[i];
    }
    proc->name[i] = 0;

    void *user_stack_base = request_pages(&global_allocator, 256);
    void *kernel_stack_base = request_pages(&global_allocator, 256);
    proc->user_stack_top = (uint8_t *)user_stack_base + (256ull * 0x1000);
    proc->kernel_stack_top = (uint8_t *)kernel_stack_base + (256ull * 0x1000);

    page_table_manager_t manager = {0};
    manager.pml4_addr = request_page(&global_allocator);
    memcpy(manager.pml4_addr, global_page_table_manager->pml4_addr, sizeof(page_table_t));
    proc->page_table_manager = manager;
    proc->cr3 = (uint64_t)manager.pml4_addr;

    uint64_t ustart = (uint64_t)proc->user_stack_top - (256ull * 0x1000);
    for (uint64_t p = ustart; p < (uint64_t)proc->user_stack_top; p += 0x1000)
    {
        uint64_t v = (p - ustart) + (256ull * 0x1000);
        map_memory(&manager, (void *)v, (void *)p, true, true);
    }

    uint64_t kstart = (uint64_t)proc->kernel_stack_top - (256ull * 0x1000);
    for (uint64_t p = kstart; p < (uint64_t)proc->kernel_stack_top; p += 0x1000)
    {
        uint64_t v = p;
        map_memory(&manager, (void *)v, (void *)p, true, false);
    }

    proc->tf = (trapframe_t *)((uint8_t *)proc->kernel_stack_top - sizeof(trapframe_t));
    memset(proc->tf, 0, sizeof(*proc->tf));

    proc->tf->rip = (uint64_t)entry;
    proc->tf->cs = UCODE_SEL;
    proc->tf->rflags = 0x202;
    proc->tf->rsp = (uint64_t)proc->user_stack_top;
    proc->tf->ss = UDATA_SEL;

    proc->tf->rdi = 0;
    proc->tf->rsi = 0;

    proc->entry = entry;
    proc->state = PROC_READY;
    proc->next = NULL;

    if (!base_proc)
    {
        base_proc = proc;
    }
}

void start_proc(proccess_t *proc)
{
    proc->state = PROC_READY;
    if (current_proc)
    {
        proccess_t *p = current_proc;
        while (p->next != 0)
        {
            p = p->next;
        }
        p->next = proc;
    }
    else
    {
        current_proc = proc;
    }
}

void scheduler_tick(trapframe_t *tf)
{
    if (!current_proc)
        return;

    *current_proc->tf = *tf;
    current_proc = current_proc->next ? current_proc->next : base_proc;

    enter_userspace(current_proc->cr3, current_proc->tf);
}