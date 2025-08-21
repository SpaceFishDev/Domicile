#include "kernel_util.h"
#include "../gdt/gdt.h"
#include "../interrupts/idt.h"
#include "../interrupts/interrupts.h"
#include "../device-drivers/ps2/keyboard.h"
#include "../device-drivers/ps2/mouse.h"
#include "../device-drivers/acpi/acpi.h"
#include "../device-drivers/pci/pci.h"
#include "../kernel-trace/kernel_trace.h"
#include "../device-drivers/pit/pit.h"
#include "../device-drivers/ahci/ahci.h"
#include "../filesystem/fat.h"
#include "../filesystem/filesystem.h"
#include "../renderer/renderer.h"
#include "../memdebug/memdebug.h"
#include "../syscalls/syscall.h"
#include "../scheduler/scheduler.h"

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
        map_memory(&page_table_manager, (void *)t, (void *)t, true, false);
    }

    uint64_t fb_base = (uint64_t)boot_info->frame_buffer->base_addr;
    uint64_t fb_size = (uint64_t)boot_info->frame_buffer->buffer_size + 4096;

    lock_pages(&global_allocator, (void *)fb_base, fb_size / 4096 + 1);

    for (uint64_t t = fb_base; t < (fb_base + fb_size); t += 0x1000)
    {
        map_memory(&page_table_manager, (void *)t, (void *)t, false, false);
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
    KERN_TRACE_FUNC;

    idtr.limit = 0x0FFF;
    idtr.offset = (uint64_t)request_page(&global_allocator);

    // CPU exceptions
    set_idt_gate(divide_error_handler, 0x00, IDT_TA_InterruptGate, 0x08);             // #DE
    set_idt_gate(debug_handler, 0x01, IDT_TA_InterruptGate, 0x08);                    // #DB
    set_idt_gate(breakpoint_handler, 0x03, IDT_TA_InterruptGate, 0x08);               // #BP
    set_idt_gate(overflow_handler, 0x04, IDT_TA_InterruptGate, 0x08);                 // #OF
    set_idt_gate(bound_range_exceeded_handler, 0x05, IDT_TA_InterruptGate, 0x08);     // #BR
    set_idt_gate(invalid_opcode_handler, 0x06, IDT_TA_InterruptGate, 0x08);           // #UD
    set_idt_gate(device_not_available_handler, 0x07, IDT_TA_InterruptGate, 0x08);     // #NM
    set_idt_gate(double_fault_handler, 0x08, IDT_TA_InterruptGate, 0x08);             // #DF
    set_idt_gate(invalid_tss_handler, 0x0A, IDT_TA_InterruptGate, 0x08);              // #TS
    set_idt_gate(segment_not_present_handler, 0x0B, IDT_TA_InterruptGate, 0x08);      // #NP
    set_idt_gate(stack_segment_fault_handler, 0x0C, IDT_TA_InterruptGate, 0x08);      // #SS
    set_idt_gate(general_protection_handler, 0x0D, IDT_TA_InterruptGate, 0x08);       // #GP
    set_idt_gate(page_fault_handler, 0x0E, IDT_TA_InterruptGate, 0x08);               // #PF
    set_idt_gate(x87_floating_point_handler, 0x10, IDT_TA_InterruptGate, 0x08);       // #MF
    set_idt_gate(alignment_check_handler, 0x11, IDT_TA_InterruptGate, 0x08);          // #AC
    set_idt_gate(machine_check_handler, 0x12, IDT_TA_InterruptGate, 0x08);            // #MC
    set_idt_gate(simd_floating_point_handler, 0x13, IDT_TA_InterruptGate, 0x08);      // #XM
    set_idt_gate(virtualization_exception_handler, 0x14, IDT_TA_InterruptGate, 0x08); // #VE
    set_idt_gate(syscall_handler, 0x80, IDT_TA_InterruptGate, 0x08);

    set_idt_gate(pit_irq_stub, 0x20, IDT_TA_InterruptGate, 0x08);
    set_idt_gate(ps2_keyboard_handler, 0x21, IDT_TA_InterruptGate, 0x08);
    set_idt_gate(ps2_mouse_handler, 0x2C, IDT_TA_InterruptGate, 0x08);

    asm("lidt %0" ::"m"(idtr));

    remap_pic();
}

basic_renderer_t *global_basic_renderer;
basic_renderer_t renderer;

mouse_handler_t mouse_handler;
#define KMALLOC_MAX_DESCRIPTORS 16384

#define DEVICE(device_id, vendor_id, name) \
    ((pci_device_t){device_id, vendor_id, name})

void register_known_devices()
{
    register_device(DEVICE(0x2930, 0x8086, "SMBus Controller"));
    register_device(DEVICE(0x2922, 0x8086, "SATA Controller [AHCI mode]"));
    register_device(DEVICE(0x2918, 0x8086, "LPC Interface Controller"));
    register_device(DEVICE(0x29C0, 0x8086, "Express DRAM Controller"));
    register_device(DEVICE(0x1111, 0x1234, "VGA Controller"));
}

void init_gdt()
{
    gdt_descriptor_t gdt_descriptor;
    gdt_descriptor.size = sizeof(gdt_t) - 1;
    gdt_descriptor.offset = (uint64_t)&default_gdt;
    load_gdt(&gdt_descriptor);
}

void init_memory(kernel_info_t *kernel_info, boot_info_t *boot_info)
{
    prepare_memory(boot_info);
    kernel_info->page_table_manager = &page_table_manager;
    global_page_table_manager = kernel_info->page_table_manager;
    init_allocator(&global_kmalloc);
}

void init_rendering(boot_info_t *boot_info)
{
    renderer = (basic_renderer_t){point(40, 40), boot_info->frame_buffer, boot_info->font};
    global_basic_renderer = &renderer;
}

void init_keyboard()
{
    key_event_t *buffer = (key_event_t *)request_page(&global_allocator);
    init_keyboard_handler(&global_keyboard_handler, buffer, 0x1000 / sizeof(key_event_t));
}

void init_mouse()
{
    mouse_event_t *mouse_buffer = (mouse_event_t *)request_page(&global_allocator);
    init_mouse_handler(&mouse_handler, mouse_buffer, 0x1000);
    global_mouse_handler = &mouse_handler;
}

void unmask_pic()
{
    outb(PIC1_DATA, 0b11111000);
    outb(PIC2_DATA, 0b11101111);
}

void init_pci(boot_info_t *boot_info)
{
    register_known_devices();
    mcfg_header_t *mcfg_header = prepare_acpi(boot_info->rsdp);
    enumerate_pci(mcfg_header);
}

void init_ahci(kernel_info_t *kernel_info)
{
    ahci_manager_t *manager = malloc(sizeof(ahci_manager_t));
    if (AHCI_exists)
    {
        for (uint64_t i = 0; i < sizeof(ahci_manager_t); ++i)
        {
            char *ptr = (char *)manager;
            ptr[i] = 0;
        }
        init_ahci_manager(manager, ahci_device);
    }
    else
    {
        free(manager);
        manager = 0;
    }
    kernel_info->ahci_manager = manager;
}

void init_fs()
{
    fat32_manager_t *f32 = malloc(sizeof(fat32_manager_t));
    init_fat32_manager(f32, 0);

    file_system_t *file_system = malloc(sizeof(file_system_t));
    init_file_system(file_system, 0);

    global_file_system = file_system;

    file_system_manager_t *f32_manager = malloc(sizeof(file_system_manager_t));
    *f32_manager = fat_create_fs_manager(f32);

    register_fs_manager(f32_manager);
}

void init_kernel(kernel_info_t *kernel_info, boot_info_t *boot_info)
{
    init_gdt();
    init_tss();
    init_memory(kernel_info, boot_info);
    init_memdebug();
    init_rendering(boot_info);

    init_kernel_trace();
    global_trace_manager->logging = 1;

    init_keyboard();

    init_syscalls();

    prepare_interrupts();

    unmask_pic();

    pit_set_divisor(10000);
    asm("sti");

    clear_screen(global_basic_renderer, 0, 0, 0);
    init_pci(boot_info);

    init_ahci(kernel_info);

    init_fs();
    init_renderer(boot_info->frame_buffer->width, boot_info->frame_buffer->height, boot_info->frame_buffer->base_addr, 1024 * 4);
    bool font_inited = set_current_font("default");

    dump_trace();
    printf("Kernel initialized successfully\n");

    global_trace_manager->logging = 0;
    kernel_trace_clear();
    memdebug_view();
    cleanup_memdebug();
}