#include "utils/kernel_util.h"
#include "interrupts/panic.h"
#include "io/io.h"
#include "ps2/keyboard.h"

void _start(boot_info_t *boot_info_ptr)
{
    kernel_info_t kernel_info;
    init_kernel(&kernel_info, boot_info_ptr);
    clear_screen(global_basic_renderer, 0, 0, 0);
    printf("Kernel initialized successfully\n");
    char *t = malloc(4096);
    char *str = malloc(200);
    // int alpha = 'a';
    // for (int i = 0; i < 200; ++i)
    // {
    //     str[i] = alpha;
    //     if (alpha == 'z')
    //     {
    //         alpha = 'a';
    //     }
    //     ++alpha;
    // }
    // str[199] = 0;
    // printf("%s\n", str);
    // char buf[32];
    // uitoa((uint64_t)str, buf);
    // printf("ptr str: %s\n", buf);
    // uitoa((uint64_t)t, buf);
    // printf("ptr t: %s\n", buf);
    while (true)
    {
        key_event_t ev = pop_key_event(&global_keyboard_handler);
        if (ev.type == KEY_PRESS)
        {
            printf("%c", ev.key);
        }
    }
}
