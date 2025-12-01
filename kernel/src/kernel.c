#include "utils/kernel_util.h"
#include "interrupts/panic.h"
#include "io/io.h"
#include "device-drivers/ps2/keyboard.h"
#include "device-drivers/ps2/mouse.h"
#include "kernel-trace/kernel_trace.h"
#include "device-drivers/pit/pit.h"
#include "filesystem/filesystem.h"
#include "renderer/renderer.h"
#include "math/math.h"
#include "text_renderer/text_renderer.h"
#include "syscalls/syscall.h"
#include "executables/vm.h"

void _start(boot_info_t *boot_info_ptr)
{
    kernel_info_t kernel_info;
    init_kernel(&kernel_info, boot_info_ptr);
    KERN_TRACE("Kernel Initialized.");

    texture_t *background = create_texture(boot_info_ptr->frame_buffer->width, boot_info_ptr->frame_buffer->height);
    uint64_t bg_id = renderer_add_texture(background);
    background->should_clear = false;

    for (int x = 0; x < background->bounds.w; ++x)
    {
        for (int y = 0; y < background->bounds.h; ++y)
        {
            float r = (float)x / (float)background->bounds.w;
            r *= 255.0f;
            uint8_t r8 = (uint8_t)r;
            float g = (float)y / (float)background->bounds.h;
            g *= 255.0f;
            uint8_t g8 = (uint8_t)g;

            texture_put_pixel(background, VEC2(x, y), COLOR(r8, g8, 150, 255));
        }
    }
    texture_t *tex = render_text_to_texture("Welcome to domicile.", 34, COLOR(255, 255, 255, 1));
    tex->bounds.x = 25;
    tex->bounds.y = 25;
    renderer_add_texture(tex);

    renderer_clear();
    renderer_draw();

    while (true)
    {
        asm("hlt");
    }
}
