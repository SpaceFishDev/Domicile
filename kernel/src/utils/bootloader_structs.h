
#ifndef BOOTLOADER_STRUCTS_H
#define BOOTLOADER_STRUCTS_H

#include "efi_memory.h"

typedef struct
{
    void *base_addr;
    size_t buffer_size;
    unsigned int width, height;
    unsigned int pixels_per_scanline;
} frame_buffer_t;

typedef struct
{
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charsize;
} psf1_header_t;

typedef struct
{
    psf1_header_t *psf1_header;
    void *glyph_buffer;

} psf1_font_t;

typedef struct
{
    void *map;
    uint64_t map_size;
    uint64_t map_descriptor_size;
} mem_info_t;

typedef struct
{
    frame_buffer_t *frame_buffer;
    psf1_font_t *font;
    mem_info_t *memory_info;
} boot_info_t;

#endif