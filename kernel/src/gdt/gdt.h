#ifndef GDT_H

#define GDT_H

#include <stdint.h>

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

typedef struct __attribute__((packed)) __attribute__((aligned(0x1000)))
{
    gdt_entry_t null;
    gdt_entry_t kernel_code;
    gdt_entry_t kernel_data;
    gdt_entry_t user_null;
    gdt_entry_t user_code;
    gdt_entry_t user_data;
} gdt_t;
extern gdt_t default_gdt;

extern void load_gdt(gdt_descriptor_t *gdt_descriptor);

#endif