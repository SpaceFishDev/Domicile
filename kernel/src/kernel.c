#include "utils/kernel_util.h"
#include "interrupts/panic.h"
#include "io/io.h"
#include "device-drivers/ps2/keyboard.h"
#include "device-drivers/ps2/mouse.h"
#include "kernel-trace/kernel_trace.h"
#include "device-drivers/pit/pit.h"
#include "filesystem/filesystem.h"
#include "renderer/renderer.h"

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
            texture_put_pixel(background, VEC2(x, y), COLOR(60, 60, 60, 255));
        }
    }
    texture_t *test = create_texture(200, 200);
    uint64_t test_id = renderer_add_texture(test);
    test->bounds.x = 50;
    test->bounds.y = 20;
    for (int x = 0; x < 200; ++x)
    {
        for (int y = 0; y < 200; ++y)
        {
            float fx = x;
            float fy = y;
            float fb = fx / 200.0f;
            float fg = fy / 200.0f;
            fb *= 255;
            fg *= 255;
            uint8_t r = 80;
            uint8_t g = (uint8_t)fg;
            uint8_t b = (uint8_t)fb;
            texture_put_pixel(test, VEC2(x, y), COLOR(r, g, b, 255));
        }
    }

    while (true)
    {
        renderer_clear();
        for (int x = 0; x < 200; ++x)
        {
            for (int y = 0; y < 200; ++y)
            {
            }
        }
        renderer_draw();
    }
}
