#include "basic_renderer.h"

void framebuffer_put_pixel(frame_buffer_t *frame_buffer, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    int *ptr = (int *)frame_buffer->base_addr;
    ptr = ptr + (x + y * frame_buffer->pixels_per_scanline);
    char *chptr = (char *)ptr;
    chptr[0] = b;
    chptr[1] = g;
    chptr[2] = r;
    chptr[3] = 255;
}

// todo: rewrite this to use framebuffer_put_pixel
void draw_char(frame_buffer_t *frame_buffer, psf1_font_t *font, unsigned int color, char chr, unsigned int _x, unsigned int _y)
{
    int *pix_ptr = (int *)frame_buffer->base_addr;
    char *font_ptr = font->glyph_buffer + (chr * font->psf1_header->charsize);
    for (unsigned int y = _y; y < _y + 16; ++y)
    {
        for (unsigned int x = _x; x < _x + 8; ++x)
        {
            if ((*font_ptr & (0b10000000 >> (x - _x))) > 0)
            {
                *((unsigned int *)(pix_ptr + x + y * frame_buffer->pixels_per_scanline)) = color;
            }
        }
        font_ptr++;
    }
}

void print_str(basic_renderer_t *renderer, char *str, uint32_t color)
{
    while (*str)
    {
        if (*str == '\n')
        {
            renderer->cursor_position.y += 16;
            renderer->cursor_position.x = 40;
            ++str;
        }
        else
        {
            draw_char(renderer->frame_buffer, renderer->font, color, *str, renderer->cursor_position.x, renderer->cursor_position.y);
            renderer->cursor_position.x += 8;
            ++str;
        }
    }
}

void clear_screen(basic_renderer_t *renderer)
{
    for (uint32_t x = 0; x < renderer->frame_buffer->width; ++x)
    {
        for (uint32_t y = 0; y < renderer->frame_buffer->height; ++y)
        {
            framebuffer_put_pixel(renderer->frame_buffer, x, y, 0, 0, 0);
        }
    }
}
