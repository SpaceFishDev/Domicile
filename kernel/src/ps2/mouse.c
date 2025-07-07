#include "mouse.h"
#include "../basic_renderer/basic_renderer.h"
#include "../memory/malloc.h"

mouse_handler_t global_mouse_handler;

void mouse_wait()
{
    uint64_t timeout = 100000;
    while (timeout--)
    {
        if ((inb(0x64) & 0b10) == 0)
        {
            return;
        }
    }
}

void mouse_wait_input()
{
    uint64_t timeout = 100000;
    while (timeout--)
    {
        if (inb(0x64) & 0b1)
        {
            return;
        }
    }
}

void mouse_write(uint8_t value)
{
    mouse_wait();
    outb(0x64, 0xD4);
    mouse_wait();
    outb(0x60, value);
}

uint8_t mouse_read()
{
    mouse_wait_input();
    return inb(0x60);
}
void init_mouse_handler(mouse_handler_t *handler)
{
    handler->current_cycle = 0;
    memset(handler->packet, 0, 4);
    handler->ready = 0;
    handler->buffer = malloc(sizeof(mouse_event_t));
    handler->buffer_size = 1;
}

void init_ps2_mouse()
{
    outb(0x64, 0xA8); // enabling the auxiliary device - mouse

    mouse_wait();
    outb(0x64, 0x20); // tells the keyboard controller that we want to send a command to the mouse
    mouse_wait_input();
    uint8_t status = inb(0x60);
    status |= 0b10;
    mouse_wait();
    outb(0x64, 0x60);
    mouse_wait();
    outb(0x60, status); // setting the correct bit is the "compaq" status byte

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();
}

void handle_ps2_mouse_data(uint8_t data)
{
    switch (global_mouse_handler.current_cycle)
    {
    case 0:
    {
        if (global_mouse_handler.ready)
        {
            break;
        }
        if ((data & 0b00001000) == 0)
        {
            break;
        }
        global_mouse_handler.packet[0] = data;
        global_mouse_handler.current_cycle++;
    }
    break;
    case 1:
    {
        if (global_mouse_handler.ready)
        {
            break;
        }
        global_mouse_handler.packet[1] = data;
        global_mouse_handler.current_cycle++;
    }
    break;
    case 2:
    {
        if (global_mouse_handler.ready)
        {
            break;
        }
        global_mouse_handler.packet[2] = data;
        global_mouse_handler.ready = 1;
        global_mouse_handler.current_cycle = 0;
    }
    break;
    }
}

#include <stdbool.h>

void process_mouse_packet()
{
    if (!global_mouse_handler.ready)
    {
        return;
    }
    global_mouse_handler.ready = 0;

    bool x_neg, y_neg, x_overflow, y_overflow;
    uint8_t *packet = global_mouse_handler.packet;
    mouse_event_t ev;
    x_neg = (packet[0] & PS2XSign) > 0;
    y_neg = (packet[0] & PS2YSign) > 0;
    x_overflow = (packet[0] & PS2XOverflow) > 0;
    y_overflow = (packet[0] & PS2YOverflow) > 0;
    if (!x_neg)
    {
        ev.type = X_MOVEMENT;
        ev.value = packet[1];
        ev.value += x_overflow ? 255 : 0;
    }
    if (x_neg)
    {
        ev.type = X_MOVEMENT;
        ev.value = -packet[1];
        ev.value -= x_overflow ? 255 : 0;
    }
    if (!y_neg)
    {
        ev.type = Y_MOVEMENT;
        ev.value = packet[2];
        ev.value += y_overflow ? 255 : 0;
    }
    if (y_neg)
    {
        ev.type = Y_MOVEMENT;
        ev.value = -packet[2];
        ev.value -= y_overflow ? 255 : 0;
    }
    push_mouse_event(ev);
}
void push_mouse_event(mouse_event_t ev)
{
    global_mouse_handler.buffer[global_mouse_handler.buffer_size] = ev;
    global_mouse_handler.buffer_size++;
    global_mouse_handler.buffer = realloc(global_mouse_handler.buffer, global_mouse_handler.buffer_size * sizeof(mouse_event_t));
}
mouse_event_t pop_mouse_event()
{
    if (global_mouse_handler.buffer_size == 0)
    {
        return (mouse_event_t){};
    }
    global_mouse_handler.buffer_size--;
    return global_mouse_handler.buffer[global_mouse_handler.buffer_size];
}