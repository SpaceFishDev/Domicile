#include "utils/kernel_util.h"
#include "interrupts/panic.h"
#include "io/io.h"
#include "ps2/keyboard.h"
#include "ps2/mouse.h"

void _start(boot_info_t *boot_info_ptr)
{
    kernel_info_t kernel_info;
    init_kernel(&kernel_info, boot_info_ptr);
    clear_screen(global_basic_renderer, 0, 0, 0);
    printf("Kernel initialized successfully\n");
    char *t = malloc(4096);
    char *str = malloc(200);
    int mouse_x = 0;
    int mouse_y = 0;
    while (true)
    {
        key_event_t ev = pop_key_event(&global_keyboard_handler);
        process_mouse_packet();
        mouse_event_t mouse_ev = pop_mouse_event();
        if (mouse_ev.type != MOUSE_NO_TYPE)
        {
            printf("Mouse Ev\n");
        }
        if (ev.type == KEY_PRESS)
        {
            printf("%c", ev.key);
        }
        if (mouse_ev.type == X_MOVEMENT)
        {
            printf("Here\n ");
            mouse_x += mouse_ev.value;
        }
        if (mouse_ev.type == Y_MOVEMENT)
        {
            mouse_y -= mouse_ev.value;
        }
        framebuffer_put_pixel(global_basic_renderer->frame_buffer, mouse_x, mouse_y, 255, 255, 255);
    }
}
