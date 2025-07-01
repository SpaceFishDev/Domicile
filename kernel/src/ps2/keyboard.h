#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

extern char qwerty_ascii_table[];
char translate_qwerty_scancode(uint8_t scancode, bool upper_case);

typedef enum
{
    PRESS,
    RELEASE,
    SPECIAL,
} key_event_type;

typedef struct
{
    key_event_type type;
    uint8_t key;
} key_event_t;

typedef struct
{
    key_event_t buffer[512];
    uint64_t keyboard_queue_pos;
    bool shift_pressed;
    bool cntrl_pressed;
} keyboard_handler_t;

#endif