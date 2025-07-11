#include "panic.h"
#include "../kernel-trace/kernel_trace.h"

void panic(char *panic_message)
{
    clear_screen(global_basic_renderer, 255, 0, 10);
    printf("KERNEL PANIC: %s\n", panic_message);
    dump_trace();
}