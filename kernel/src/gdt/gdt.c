#include "gdt.h"

__attribute__((aligned(0x1000)))
gdt_t default_gdt = {
    {0, 0, 0, 0x00, 0x00, 0}, // null
    {0, 0, 0, 0x9a, 0xa0, 0}, // kernel code segment
    {0, 0, 0, 0x92, 0xa0, 0}, // kernel data segment
    {0, 0, 0, 0x00, 0x00, 0}, // user null
    {0, 0, 0, 0xFA, 0xA0, 0}, // user code (ring 3, execute/read)
    {0, 0, 0, 0xF2, 0xA0, 0}, // user data (ring 3, read/write)
    {0},
};

tss64_t default_tss = {
    .reserved0 = 0,
    .rsp0 = (uint64_t)(kernel_stack + sizeof(kernel_stack)), // top of stack
    .rsp1 = 0,
    .rsp2 = 0,
    .reserved1 = 0,
    .ist1 = 0, // optional extra stacks
    .ist2 = 0,
    .ist3 = 0,
    .ist4 = 0,
    .ist5 = 0,
    .ist6 = 0,
    .ist7 = 0,
    .reserved2 = 0,
    .reserved3 = 0,
    .iopb_offset = sizeof(tss64_t) // deny user-mode port I/O
};

#include "../utils/string.h"

void init_tss()
{
    set_tss_descriptor(&default_gdt.tss, (uint64_t)&default_tss, sizeof(default_tss) - 1);
    uint16_t tss_selector = 0x30;
    asm volatile("ltr %0" : : "r"(tss_selector));
}

void set_tss_descriptor(tss_descriptor_t *desc, uint64_t base, uint32_t limit)
{
    memset(desc, 0, sizeof(*desc));
    desc->limit0 = limit & 0xFFFF;
    desc->base0 = base & 0xFFFF;
    desc->base1 = (base >> 16) & 0xFF;
    desc->access = 0x89; // Present, DPL=0, Type=9 (Available 64-bit TSS)
    desc->limit1_flags = ((limit >> 16) & 0x0F);
    desc->base2 = (base >> 24) & 0xFF;
    desc->base3 = (base >> 32) & 0xFFFFFFFF;
}
