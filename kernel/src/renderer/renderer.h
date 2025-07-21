#ifndef RENDERER_H

#define RENDERER_H

#include <stdint.h>
#include <stddef.h>
#include "../utils/kernel_util.h"

void memcpy32_fast(void *dst, const void *src, size_t dwords);

typedef struct
{
    int x, y, w, h;
} rect_t;

typedef struct
{
    rect_t bounds;
    bool should_clear;
    bool drawn;
    uint32_t *pixel_buffer;
    bool changed;
} texture_t;

typedef struct
{
    uint8_t r, g, b, a;
} color_t;

typedef struct
{
    int64_t x;
    int64_t y;
} vector2_t;

typedef struct
{
    uint32_t *frame_buffer;
    uint64_t num_tex;
    texture_t **textures;
    int w, h;
} renderer_t;

void init_renderer(uint64_t w, uint64_t h, void *fb_base_addr, uint64_t max_textures);
uint64_t renderer_add_texture(texture_t *tex);
void renderer_delete_texture(uint64_t id);
void renderer_draw();
void renderer_clear();
void free_renderer();
texture_t *create_texture(uint64_t width, uint64_t height);
void texture_put_pixel(texture_t *tex, vector2_t pos, color_t pix);

#define COLOR(r, g, b, a) ((color_t){r, g, b, a})
#define VEC2(x, y) ((vector2_t){x, y})

extern renderer_t *global_renderer;

#endif