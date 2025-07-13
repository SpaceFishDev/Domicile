#include "utils/kernel_util.h"
#include "interrupts/panic.h"
#include "io/io.h"
#include "device-drivers/ps2/keyboard.h"
#include "device-drivers/ps2/mouse.h"
#include "kernel-trace/kernel_trace.h"
#include "device-drivers/pit/pit.h"

void _start(boot_info_t *boot_info_ptr)
{
    kernel_info_t kernel_info;
    init_kernel(&kernel_info, boot_info_ptr);
    KERN_TRACE("Kernel Initialized.");
    void *ptr = request_pages(&global_allocator, 2000);
    printf("Free Mem: %u MB\n", get_free_memory() / (1024 * 1024));
    while (true)
    {
        asm("hlt");
    }
}
