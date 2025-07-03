#include "keyboard.h"
#include "../basic_renderer/basic_renderer.h"

keyboard_handler_t global_keyboard_handler;

char qwerty_ascii_table[] = {
    0,
    0,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    0,
    0,
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    0,
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
};

char translate_qwerty_scancode(uint8_t scancode, bool upper_case)
{
    if (scancode > 58)
        return 0;
    return qwerty_ascii_table[scancode];
}

void init_keyboard_handler(keyboard_handler_t *handler, key_event_t *buffer, uint64_t buffer_size)
{
    handler->shift_pressed = false;
    handler->cntrl_pressed = false;
    handler->keyboard_stack_pos = buffer_size - 1;
    handler->buffer = buffer;
    handler->buffer_size = buffer_size;
}

void create_key_event(key_event_t *ev, key_event_type type, uint8_t key)
{
    ev->key = key;
    ev->type = type;
}

key_event_t pop_key_event(keyboard_handler_t *handler)
{
    if (handler->keyboard_stack_pos > (handler->buffer_size - 1))
    {
        key_event_t ev;
        create_key_event(&ev, KEY_NONE, 0);
        return ev;
    }
    handler->keyboard_stack_pos++;
    key_event_t ev = handler->buffer[handler->keyboard_stack_pos];
    return ev;
}

void push_key_event(keyboard_handler_t *handler, key_event_t ev)
{
    if (handler->keyboard_stack_pos <= 0)
    {
        handler->keyboard_stack_pos = handler->buffer_size - 1;
    }
    handler->buffer[handler->keyboard_stack_pos] = ev;
    handler->keyboard_stack_pos--;
}

bool is_key_release(uint8_t scancode)
{
    return scancode & 0x80;
}

bool is_key_special(uint8_t scancode)
{
    return (scancode == PS2_BACK_SPACE) || (scancode == PS2_ENTER) || (scancode == PS2_LEFT_SHIFT) || (scancode == PS2_RIGHT_SHIFT);
}

void handle_key(keyboard_handler_t *handler, uint8_t scancode)
{
    key_event_t ev;
    uint8_t ascii = translate_qwerty_scancode(scancode, false);
    if (!ascii)
    {
        return;
    }
    if (!is_key_release(scancode))
    {
        create_key_event(&ev, KEY_PRESS, ascii);
    }
    else
    {
        create_key_event(&ev, KEY_RELEASE, ascii);
    }
    push_key_event(handler, ev);
}