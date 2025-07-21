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

    bool font_inited = set_current_font("/sans.ttf");
    if (!font_inited)
    {
        global_basic_renderer->disabled = false;
        printf(":( font died\n");
        global_basic_renderer->disabled = true;
        while (true)
        {
            asm("hlt");
        }
    }

    texture_t *tex = render_text_to_texture("Hello, World", 32);
    renderer_add_texture(tex);

    while (true)
    {
        renderer_clear();
        renderer_draw();
    }
    // never happens but sanity or something idk gang
    renderer_delete_texture(bg_id);
}
