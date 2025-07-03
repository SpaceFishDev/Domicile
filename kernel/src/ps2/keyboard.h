#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

extern char qwerty_ascii_table[];
char translate_qwerty_scancode(uint8_t scancode, bool upper_case);

#define PS2_LEFT_SHIFT 0x2A
#define PS2_RIGHT_SHIFT 0x36
#define PS2_ENTER 0x1C
#define PS2_BACK_SPACE 0x0E

typedef enum
{
    KEY_PRESS,
    KEY_RELEASE,
    KEY_SPECIAL_PRESS,
    KEY_SPECIAL_RELEASE,
    KEY_NONE,
} key_event_type;

typedef struct
{
    key_event_type type;
    uint8_t key;
} key_event_t;

typedef struct
{
    key_event_t *buffer;
    uint64_t keyboard_stack_pos;
    bool shift_pressed;
    bool cntrl_pressed;
    uint64_t buffer_size;
} keyboard_handler_t;

void init_keyboard_handler(keyboard_handler_t *handler, key_event_t *buffer, uint64_t buffer_size);
void create_key_event(key_event_t *ev, key_event_type type, uint8_t key);
key_event_t pop_key_event(keyboard_handler_t *handler);
void push_key_event(keyboard_handler_t *handler, key_event_t ev);
void handle_key(keyboard_handler_t *handler, uint8_t scancode);

extern keyboard_handler_t global_keyboard_handler;

#endif