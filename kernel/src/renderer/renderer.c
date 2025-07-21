#include "renderer.h"
#include "../kernel-trace/kernel_trace.h"

void memcpy32_fast(void *dst, const void *src, size_t count)
{
    size_t qwords = count / 2;

    register uint64_t *rdi asm("rdi") = dst;
    register const uint64_t *rsi asm("rsi") = src;
    register size_t rcx asm("rcx") = qwords;

    asm volatile(
        "rep movsq"
        : "+D"(rdi), "+S"(rsi), "+c"(rcx)
        :
        : "memory");

    if (count & 1)
    {
        ((uint32_t *)dst)[count & ~1] = ((const uint32_t *)src)[count & ~1];
    }
}
void memset32_fast(void *dst, uint32_t value, size_t count)
{
    register uint32_t eax asm("eax") = value;
    register uint32_t *edi asm("rdi") = (uint32_t *)dst;
    register size_t ecx asm("rcx") = count;

    asm volatile(
        "rep stosl"
        : "+D"(edi), "+c"(ecx), "+a"(eax)
        :
        : "memory");
}

renderer_t *global_renderer;

void init_renderer(uint64_t w, uint64_t h, void *fb_base_addr, uint64_t max_textures)
{
    KERN_TRACE_FUNC;
    global_renderer = malloc(sizeof(renderer_t));
    global_renderer->w = w;
    global_renderer->h = h;
    global_renderer->frame_buffer = fb_base_addr;
    global_renderer->textures = malloc(max_textures * sizeof(texture_t *));
    global_renderer->num_tex = max_textures;
    for (int i = 0; i < global_renderer->num_tex; ++i)
    {
        global_renderer->textures[i] = 0;
    }
    global_basic_renderer->disabled = true;
}

texture_t *create_texture(uint64_t width, uint64_t height)
{
    texture_t *tex = malloc(sizeof(texture_t));
    tex->bounds.w = width;
    tex->bounds.h = height;
    tex->bounds.x = 0;
    tex->bounds.y = 0;
    tex->should_clear = true;
    tex->pixel_buffer = malloc(width * height * sizeof(uint32_t));
    tex->drawn = false;
    return tex;
}

uint64_t renderer_add_texture(texture_t *tex)
{
    for (uint64_t i = 0; i < global_renderer->num_tex; ++i)
    {
        if (global_renderer->textures[i] == 0)
        {
            global_renderer->textures[i] = tex;
            return i;
        }
    }
    return -1;
}

void renderer_delete_texture(uint64_t id)
{
    free(global_renderer->textures[id]->pixel_buffer);
    free(global_renderer->textures[id]);
    global_renderer->textures[id] = 0;
}

void renderer_clear()
{
    for (int i = 0; i < global_renderer->num_tex; ++i)
    {
        if (global_renderer->textures[i] != 0)
        {
            if (global_renderer->textures[i]->should_clear && global_renderer->textures[i]->changed)
            {
                texture_t *tex = global_renderer->textures[i];
                uint64_t x = tex->bounds.x;
                uint64_t start_y = tex->bounds.y;
                uint64_t y_lim = start_y + tex->bounds.h;
                for (uint64_t y = start_y; y < y_lim; ++y)
                {
                    uint32_t *dst = &global_renderer->frame_buffer[x + (y * global_renderer->w)];
                    memset32_fast(dst, 0, tex->bounds.w);
                    tex->drawn = false;
                }
            }
        }
    }
}

void renderer_draw()
{
    for (int i = 0; i < global_renderer->num_tex; ++i)
    {
        if (global_renderer->textures[i] != 0)
        {
            texture_t *tex = global_renderer->textures[i];
            if (tex->drawn == true && !tex->should_clear)
            {
                continue;
            }
            if (!tex->drawn && tex->changed)
            {
                uint64_t x = tex->bounds.x;
                uint64_t start_y = tex->bounds.y;
                for (uint64_t y = start_y; y < (start_y + tex->bounds.h); ++y)
                {
                    uint32_t *dst = &global_renderer->frame_buffer[x + (y * global_renderer->w)];
                    uint32_t *src = &tex->pixel_buffer[(y - start_y) * tex->bounds.w];
                    memcpy32_fast(dst, src, tex->bounds.w);
                }
                tex->drawn = true;
                tex->changed = false;
            }
        }
    }
}

void free_renderer()
{
    free(global_renderer->textures);
}

#define RGBA_TO_BGRA32(r, g, b, a) \
    (((uint32_t)(b)) |             \
     ((uint32_t)(g) << 8) |        \
     ((uint32_t)(r) << 16) |       \
     ((uint32_t)(a) << 24))

void texture_put_pixel(texture_t *tex, vector2_t pos, color_t pix)
{
    if (pos.x > tex->bounds.w || pos.x < 0)
    {
        return;
    }
    if (pos.y > tex->bounds.h || pos.y < 0)
    {
        return;
    }
    uint64_t offset = pos.x + (pos.y * tex->bounds.w);
    uint32_t color = RGBA_TO_BGRA32(pix.r, pix.g, pix.b, pix.a);
    tex->pixel_buffer[offset] = color;
    tex->changed = true;
}