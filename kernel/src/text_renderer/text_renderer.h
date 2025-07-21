#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "../memory/malloc.h"
#include "../renderer/renderer.h"
#include "../math/math.h"
#include <stdint.h>

bool set_current_font(char *path);
texture_t *render_text_to_texture(char *text, float font_size);

#endif