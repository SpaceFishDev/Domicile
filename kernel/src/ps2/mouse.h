#ifndef MOUSE_H

#define MOUSE_H

#include "../io/io.h"

#define PS2Leftbutton 0b00000001
#define PS2Middlebutton 0b00000010
#define PS2Rightbutton 0b00000100
#define PS2XSign 0b00010000
#define PS2YSign 0b00100000
#define PS2XOverflow 0b01000000
#define PS2YOverflow 0b10000000

typedef enum
{
    MOUSE_NO_TYPE,
    X_MOVEMENT,
    Y_MOVEMENT,
    MOUSE_BTN,
} mouse_event_type;

typedef struct
{
    uint64_t value;
    mouse_event_type type;
} mouse_event_t;

typedef struct
{
    uint8_t current_cycle;
    uint8_t packet[4];
    uint8_t ready;
    mouse_event_t *buffer;
    uint64_t buffer_size;
} mouse_handler_t;

void init_ps2_mouse();
void handle_ps2_mouse_data(uint8_t data);
void init_mouse_handler(mouse_handler_t *mouse_handler);
void process_mouse_packet();
void push_mouse_event(mouse_event_t ev);
mouse_event_t pop_mouse_event();

extern mouse_handler_t global_mouse_handler;

#endif