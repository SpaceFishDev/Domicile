#include "utils/kernel_util.h"
#include "interrupts/panic.h"
#include "io/io.h"
#include "ps2/keyboard.h"

void _start(boot_info_t *boot_info_ptr)
{
    boot_info_t boot_info = *boot_info_ptr;
    kernel_info_t kernel_info;
    init_kernel(&kernel_info, boot_info_ptr);
    clear_screen(global_basic_renderer, 0, 0, 0);
    printf("Kernel initialized successfully\n");

    while (true)
    {
        key_event_t ev = pop_key_event(&global_keyboard_handler);
        if (ev.type == KEY_PRESS)
        {
            printf("%c", ev.key);
        }
    }
}
