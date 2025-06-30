#ifndef INTERRUPTS_H

#define INTERRUPTS_H

#include "../basic_renderer/basic_renderer.h"
#include "../io/io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

struct interrupt_frame;
__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void double_fault_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void general_protection_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void ps2_keyboard_handler(struct interrupt_frame *interrupt_frame);

void remap_pic();
void pic_end_master();
void pic_end_slave();

#endif