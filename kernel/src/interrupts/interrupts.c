#include "interrupts.h"
#include "panic.h"
#include "../device-drivers/ps2/keyboard.h"
#include "../device-drivers/ps2/mouse.h"
#include "../device-drivers/pit/pit.h"
#include "../syscalls/syscall.h"

__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Page Fault detected");
    while (1)
        ;
}
__attribute__((interrupt)) void double_fault_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Double Fault detected");
    while (1)
        ;
}
__attribute__((interrupt)) void general_protection_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("General Protection Fault detected");
    __asm__("cli");
    while (1)
        ;
}

__attribute__((interrupt)) void ps2_keyboard_handler(struct interrupt_frame *interrupt_frame)
{
    uint8_t scan_code = inb(0x60);
    handle_key(&global_keyboard_handler, scan_code);
    pic_end_master();
}
__attribute__((interrupt)) void ps2_mouse_handler(struct interrupt_frame *interrupt_frame)
{
    uint8_t val = inb(0x60);
    handle_ps2_mouse_data(global_mouse_handler, val);
    pic_end_slave();
}

__attribute__((interrupt)) void divide_error_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Divide Error detected");
    while (1)
        ;
}

__attribute__((interrupt)) void debug_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Debug Exception detected");
    while (1)
        ;
}

__attribute__((interrupt)) void breakpoint_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Breakpoint Exception detected");
    while (1)
        ;
}

__attribute__((interrupt)) void overflow_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Overflow Exception detected");
    while (1)
        ;
}

__attribute__((interrupt)) void bound_range_exceeded_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("BOUND Range Exceeded detected");
    while (1)
        ;
}

__attribute__((interrupt)) void invalid_opcode_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Invalid Opcode detected");
    while (1)
        ;
}

__attribute__((interrupt)) void device_not_available_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Device Not Available detected");
    while (1)
        ;
}

__attribute__((interrupt)) void invalid_tss_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Invalid TSS detected");
    while (1)
        ;
}

__attribute__((interrupt)) void segment_not_present_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Segment Not Present detected");
    while (1)
        ;
}

__attribute__((interrupt)) void stack_segment_fault_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Stack-Segment Fault detected");
    while (1)
        ;
}

__attribute__((interrupt)) void x87_floating_point_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("x87 Floating-Point Exception detected");
    while (1)
        ;
}

__attribute__((interrupt)) void alignment_check_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Alignment Check Exception detected");
    while (1)
        ;
}

__attribute__((interrupt)) void machine_check_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Machine Check Exception detected");
    while (1)
        ;
}

__attribute__((interrupt)) void simd_floating_point_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("SIMD Floating-Point Exception detected");
    while (1)
        ;
}

__attribute__((interrupt)) void virtualization_exception_handler(struct interrupt_frame *interrupt_frame)
{
    global_basic_renderer->disabled = false;
    panic("Virtualization Exception detected");
    while (1)
        ;
}

int tick = 0;
void pit_handler(trapframe_t *trap_frame)
{
    pit_timer_tick();
    if (tick > 10)
    {
        scheduler_tick(trap_frame);
        tick = 0;
    }
    pic_end_master();
    ++tick;
}

void remap_pic()
{
    uint8_t a1, a2;
    a1 = inb(PIC1_DATA);
    io_wait();
    a2 = inb(PIC2_DATA);
    io_wait();

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC1_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1);
    io_wait();
    outb(PIC2_DATA, a2);
    io_wait();
}

void pic_end_master()
{
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_end_slave()
{
    outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}