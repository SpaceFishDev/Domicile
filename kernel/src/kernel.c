#include "utils/kernel_util.h"
#include "interrupts/panic.h"
basic_renderer_t *global_basic_renderer;
basic_renderer_t renderer;

void _start(boot_info_t *boot_info_ptr)
{
    boot_info_t boot_info = *boot_info_ptr;
    kernel_info_t kernel_info;
    init_kernel(&kernel_info, boot_info_ptr);
    renderer = (basic_renderer_t){point(40, 40), boot_info.frame_buffer, boot_info.font};
    global_basic_renderer = &renderer;
    clear_screen(&renderer, 0, 0, 0);
    printf("Kernel initialized successfully\n");

    while (true)
        ;
}
