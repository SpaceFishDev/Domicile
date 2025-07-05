#include "kernel_util.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"
#include "../interrupts/interrupts.h"
#include "../ps2/keyboard.h"

page_table_manager_t page_table_manager;
void prepare_memory(boot_info_t *boot_info)
{
    global_allocator = (page_frame_allocator_t){};
    read_efi_memory_map(&global_allocator, *boot_info->memory_info);
    uint64_t kernel_size = (uint64_t)&_kernel_end - (uint64_t)&_kernel_start;
    uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;
    lock_pages(&global_allocator, &_kernel_start, kernel_pages);

    page_table_t *pml4 = (page_table_t *)request_page(&global_allocator);
    memset(pml4, 0, 0x1000);
    init_page_table_manager(&page_table_manager, pml4);

    for (uint64_t t = 0; t < get_memory_size(*boot_info->memory_info); t += 0x1000)
    {
        map_memory(&page_table_manager, (void *)t, (void *)t, true);
    }

    uint64_t fb_base = (uint64_t)boot_info->frame_buffer->base_addr;
    uint64_t fb_size = (uint64_t)boot_info->frame_buffer->buffer_size + 4096;

    lock_pages(&global_allocator, (void *)fb_base, fb_size / 4096 + 1);

    for (uint64_t t = fb_base; t < (fb_base + fb_size); t += 0x1000)
    {
        map_memory(&page_table_manager, (void *)t, (void *)t, false);
    }

    asm("mov %0, %%cr3" ::"r"(pml4));
}

idtr_t idtr;

void set_idt_gate(void *handler, uint8_t entry_offset, uint8_t type_attr, uint8_t selector)
{
    idt_desc_entry_t *interrupt = (idt_desc_entry_t *)(idtr.offset + entry_offset * sizeof(idt_desc_entry_t));
    set_offset(interrupt, (uint64_t)handler);
    interrupt->type_attr = type_attr;
    interrupt->selector = selector;
}

void prepare_interrupts()
{
    idtr.limit = 0x0FFF;
    idtr.offset = (uint64_t)request_page(&global_allocator);

    set_idt_gate(double_fault_handler, 0x8, IDT_TA_InterruptGate, 0x08);

    set_idt_gate(general_protection_handler, 0xD, IDT_TA_InterruptGate, 0x08);

    set_idt_gate(ps2_keyboard_handler, 0x21, IDT_TA_InterruptGate, 0x08);

    asm("lidt %0" ::"m"(idtr));

    remap_pic();
    outb(PIC1_DATA, 0b11111101);
    outb(PIC2_DATA, 0b11111111);

    asm("sti");
}

basic_renderer_t *global_basic_renderer;
basic_renderer_t renderer;

#define KMALLOC_MAX_DESCRIPTORS 16384

void init_kernel(kernel_info_t *kernel_info, boot_info_t *boot_info)
{
    gdt_descriptor_t gdt_descriptor;
    gdt_descriptor.size = sizeof(gdt_t) - 1;
    gdt_descriptor.offset = (uint64_t)&default_gdt;
    load_gdt(&gdt_descriptor);
    prepare_memory(boot_info);
    kernel_info->page_table_manager = &page_table_manager;
    key_event_t *buffer = (key_event_t *)request_page(&global_allocator);
    init_keyboard_handler(&global_keyboard_handler, buffer, 0x1000 / sizeof(key_event_t));
    prepare_interrupts();
    renderer = (basic_renderer_t){point(40, 40), boot_info->frame_buffer, boot_info->font};
    global_basic_renderer = &renderer;
    init_allocator(&global_kmalloc, KMALLOC_MAX_DESCRIPTORS);
}