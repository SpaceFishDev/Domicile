#ifndef GDT_H

#define GDT_H

#include <stdint.h>
static uint8_t kernel_stack[16384] __attribute__((aligned(16)));

typedef struct __attribute__((packed))
{
    uint16_t size;
    uint64_t offset;
} gdt_descriptor_t;

typedef struct __attribute__((packed))
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access_byte;
    uint8_t limit1_flags;
    uint8_t base2;
} gdt_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t limit0;      // bits 0..15 of limit
    uint16_t base0;       // bits 0..15 of base
    uint8_t base1;        // bits 16..23 of base
    uint8_t access;       // type (0x89 = available TSS, DPL=0, present)
    uint8_t limit1_flags; // bits 16..19 of limit + flags
    uint8_t base2;        // bits 24..31 of base
    uint32_t base3;       // bits 32..63 of base
    uint32_t reserved;    // must be zero
} tss_descriptor_t;

typedef struct __attribute__((packed)) tss64
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} tss64_t;

typedef struct __attribute__((packed)) __attribute__((aligned(0x1000)))
{
    gdt_entry_t null;
    gdt_entry_t kernel_code;
    gdt_entry_t kernel_data;
    gdt_entry_t user_null;
    gdt_entry_t user_code;
    gdt_entry_t user_data;
    tss_descriptor_t tss;
} gdt_t;

extern gdt_t default_gdt;

extern void load_gdt(gdt_descriptor_t *gdt_descriptor);
void init_tss();

#endif