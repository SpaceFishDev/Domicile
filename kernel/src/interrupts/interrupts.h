#ifndef INTERRUPTS_H

#define INTERRUPTS_H

#include "../basic_renderer/basic_renderer.h"
#include "../io/io.h"
#include "../scheduler/scheduler.h"

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
__attribute__((interrupt)) void ps2_mouse_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void divide_error_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void debug_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void breakpoint_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void overflow_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void bound_range_exceeded_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void invalid_opcode_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void device_not_available_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void invalid_tss_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void segment_not_present_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void stack_segment_fault_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void x87_floating_point_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void alignment_check_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void machine_check_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void simd_floating_point_handler(struct interrupt_frame *interrupt_frame);
__attribute__((interrupt)) void virtualization_exception_handler(struct interrupt_frame *interrupt_frame);
void pit_handler(trapframe_t *trap_frame);

void remap_pic();
void pic_end_master();
void pic_end_slave();

#endif