#include "text_renderer.h"
#include "../filesystem/filesystem.h"
void *stb_malloc(uint64_t x)
{
    return malloc(x);
}
void stb_free(void *x)
{
    free(x);
}

#define STBTT_assert
#define STBTT_assert(x) ((void)0)
#define STBTT_malloc
#define STBTT_malloc(x, u) ((void)(u), stb_malloc(x))
#define STBTT_free(x, u) ((void)(u), stb_free(x))
#define STBTT_ifloor(x) ((int)floor(x))
#define STBTT_pow(x, y) pow(x, y)
#define STBTT_fmod(x, y) fmod(x, y)
#define STBTT_fabs(x) fabs(x)
#define STBTT_floor(x) floor(x)
#define STBTT_iceil(x) ((int)ceil(x))
#define STBTT_acos(x) acos(x)
#define STBTT_sqrt(x) sqrt(x)
#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_truetype.h"

stbtt_fontinfo *font;

bool set_current_font(char *path)
{
    if (!strcmp(path, "default"))
    {
        font = malloc(sizeof(stbtt_fontinfo));
        bool worked = stbtt_InitFont(font, default_font, 0);
        return worked;
    }
    uint64_t file_size = 0;
    uint64_t fd = fs_open(path, false);
    file_size = fs_get_file_size(fd);
    char *buffer = malloc(file_size);
    fs_read(buffer, fd);
    fs_close(fd);
    font = malloc(sizeof(stbtt_fontinfo));
    bool worked = stbtt_InitFont(font, buffer, 0);
    free(buffer);
    return worked;
}

void measure_text(const char *text, float font_size, int *out_width, int *out_height)
{
    float scale = stbtt_ScaleForPixelHeight(font, font_size);

    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(font, &ascent, &descent, &line_gap);

    int width = 0;
    int max_x0 = 0, min_y0 = 0, max_y1 = 0;

    for (const char *p = text; *p; p++)
    {
        int glyph = stbtt_FindGlyphIndex(font, *p);

        int ax, lsb;
        stbtt_GetGlyphHMetrics(font, glyph, &ax, &lsb);

        int x0, y0, x1, y1;
        stbtt_GetGlyphBitmapBox(font, glyph, scale, scale, &x0, &y0, &x1, &y1);

        if (y0 < min_y0)
            min_y0 = y0;
        if (y1 > max_y1)
            max_y1 = y1;

        width += (int)(ax * scale);
    }

    if (out_width)
        *out_width = width;
    if (out_height)
        *out_height = max_y1 - min_y0;
}

texture_t *render_text_to_texture(char *text, float font_size, color_t col)
{
    int width, height;
    measure_text(text, font_size, &width, &height);
    texture_t *tex = create_texture(width, height);

    float scale = stbtt_ScaleForPixelHeight(font, font_size);
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(font, &ascent, &descent, &line_gap);
    int text_height = (int)((ascent - descent) * scale);
    int baseline = (height - text_height) / 2 + (int)(ascent * scale);

    int x = 0;
    for (const char *p = text; *p; p++)
    {
        int glyph = stbtt_FindGlyphIndex(font, *p);
        int ax, lsb;
        stbtt_GetGlyphHMetrics(font, glyph, &ax, &lsb);
        int x0, y0, x1, y1;
        stbtt_GetGlyphBitmapBox(font, glyph, scale, scale, &x0, &y0, &x1, &y1);

        int w, h;
        unsigned char *bitmap = stbtt_GetGlyphBitmap(font, scale, scale, glyph, &w, &h, 0, 0);
        for (int by = 0; by < h; by++)
        {
            for (int bx = 0; bx < w; bx++)
            {
                unsigned char a = bitmap[by * w + bx];
                int px = x + x0 + bx;
                int py = baseline + y0 + by;
                col.a = a;

                if (px >= 0 && px < width && py >= 0 && py < height)
                {
                    texture_put_pixel(tex, VEC2(px, py), col);
                }
            }
        }
        stbtt_FreeBitmap(bitmap, NULL);
        x += (int)(ax * scale);
    }
    return tex;
}
