#include "panic.h"

void panic(char *panic_message)
{
    clear_screen(global_basic_renderer, 255, 0, 10);
    global_basic_renderer->cursor_position = (point_t){40, 0};
    print_str(global_basic_renderer, "KERNEL PANIC:\n\n", 0xFFFFFFFF);
    print_str(global_basic_renderer, panic_message, 0xFFFFFFFF);
}