#include "utils/kernel_util.h"

void _start(boot_info_t *boot_info_ptr)
{
    boot_info_t boot_info = *boot_info_ptr;
    kernel_info_t kernel_info;
    init_kernel(&kernel_info, boot_info_ptr);
    basic_renderer_t renderer = (basic_renderer_t){point(40, 40), boot_info.frame_buffer, boot_info.font};
    print_str(&renderer, "test", 0xffff0000);

    while (true)
        ;
}
