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
    printf("Free Mem: %u MB\n", get_free_memory() / (1024 * 1024));
    char *t = malloc(4096);
    char *str = malloc(200);
    uint64_t mouse_x = 50;
    uint64_t mouse_y = 50;
    while (true)
    {
        key_event_t ev = pop_key_event(&global_keyboard_handler);
        process_mouse_packet(global_mouse_handler);
        mouse_event_t mouse_ev = pop_mouse_event(global_mouse_handler);
        while (mouse_ev.type != MOUSE_NO_TYPE)
        {
            if (mouse_x > global_basic_renderer->frame_buffer->width)
            {
                mouse_x = 0;
            }
            if (mouse_y > global_basic_renderer->frame_buffer->height)
            {
                mouse_y = 0;
            }
            if (ev.type == KEY_PRESS)
            {
                printf("%c", ev.key);
            }
            if (mouse_ev.type == X_MOVEMENT)
            {
                mouse_x += (uint64_t)mouse_ev.value;
            }
            if (mouse_ev.type == Y_MOVEMENT)
            {
                mouse_y += (uint64_t)mouse_ev.value;
            }
            mouse_ev = pop_mouse_event(global_mouse_handler);
        }
        draw_char(global_basic_renderer->frame_buffer, global_basic_renderer->font, 0xFFFF0000, 'A', mouse_x, mouse_y);
    }
}
