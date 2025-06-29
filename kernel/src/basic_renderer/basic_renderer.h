#ifndef BASIC_RENDERER

#define BASIC_RENDERER

#include <stdint.h>

#include "../utils/typdef.h"

typedef struct
{
    uint32_t x;
    uint32_t y;
} point_t;

#define point(x, y) ((point_t){x, y})

typedef struct
{
    point_t cursor_position;
    frame_buffer_t *frame_buffer;
    psf1_font_t *font;
} basic_renderer_t;

void framebuffer_put_pixel(frame_buffer_t *frame_buffer, int x, int y, uint8_t r, uint8_t g, uint8_t b);
void draw_char(frame_buffer_t *frame_buffer, psf1_font_t *font, unsigned int color, char chr, unsigned int _x, unsigned int _y);
void print_str(basic_renderer_t *renderer, char *str, uint32_t color);
void clear_screen(basic_renderer_t *renderer);

#endif