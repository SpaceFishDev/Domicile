#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_TA_InterruptGate 0b10001110
#define IDT_TA_CallGate 0b10001100
#define IDT_TA_TrapGate 0b10001111

typedef struct
{
    uint16_t offset0;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t ignore;
} idt_desc_entry_t;

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint64_t offset;
} idtr_t;

void set_offset(idt_desc_entry_t *entry, uint64_t offset);
uint64_t get_offset(idt_desc_entry_t *entry);

#endif