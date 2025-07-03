#include "interrupts.h"
#include "panic.h"
#include "../ps2/keyboard.h"

__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *interrupt_frame)
{
    panic("Page Fault detected");
    while (1)
        ;
}
__attribute__((interrupt)) void double_fault_handler(struct interrupt_frame *interrupt_frame)
{
    panic("Double Fault detected");
    while (1)
        ;
}
__attribute__((interrupt)) void general_protection_handler(struct interrupt_frame *interrupt_frame)
{
    panic("General Protection Fault detected");
    while (1)
        ;
}

__attribute__((interrupt)) void ps2_keyboard_handler(struct interrupt_frame *interrupt_frame)
{
    uint8_t scan_code = inb(0x60);
    handle_key(&global_keyboard_handler, scan_code);
    pic_end_master();
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