#include "mouse.h"
#include "../../basic_renderer/basic_renderer.h"
#include "../../memory/malloc.h"
#include "../../kernel-trace/kernel_trace.h"

mouse_handler_t *global_mouse_handler;

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
void init_mouse_handler(mouse_handler_t *handler, mouse_event_t *buffer, uint64_t buffer_size)
{
    KERN_TRACE_FUNC;

    init_ps2_mouse();
    handler->current_cycle = 0;
    memset(handler->packet, 0, 4);
    handler->ready = 0;
    handler->buffer = buffer;
    handler->buffer_size = buffer_size / sizeof(mouse_event_t);
    handler->mouse_stack_pos = handler->buffer_size - 1;

    if (!handler->buffer)
    {
        printf("Couldnt get buffer!\n");
    }
    printf("Buffer PTR: %u\n", handler->buffer);
}

void init_ps2_mouse()
{
    KERN_TRACE_FUNC;
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

void handle_ps2_mouse_data(mouse_handler_t *handler, uint8_t data)
{
    switch (handler->current_cycle)
    {
    case 0:
    {
        if (handler->ready)
        {
            break;
        }
        if ((data & 0b00001000) == 0)
        {
            break;
        }
        handler->packet[0] = data;
        handler->current_cycle++;
    }
    break;
    case 1:
    {
        if (handler->ready)
        {
            break;
        }
        handler->packet[1] = data;
        handler->current_cycle++;
    }
    break;
    case 2:
    {
        if (handler->ready)
        {
            break;
        }
        handler->packet[2] = data;
        handler->ready = 1;
        handler->current_cycle = 0;
    }
    break;
    }
}

#include <stdbool.h>

void process_mouse_packet(mouse_handler_t *handler)
{
    if (!handler->ready)
    {
        return;
    }
    handler->ready = 0;

    bool x_neg, y_neg, x_overflow, y_overflow;
    uint8_t *packet = handler->packet;
    mouse_event_t ev;
    ev.type = MOUSE_NO_TYPE;
    x_neg = (packet[0] & PS2XSign) > 0;
    y_neg = (packet[0] & PS2YSign) > 0;
    x_overflow = (packet[0] & PS2XOverflow) > 0;
    y_overflow = (packet[0] & PS2YOverflow) > 0;
    if (packet[0] & PS2Middlebutton)
    {
        ev.type = MOUSE_MIDDLE_BUTTON;
    }
    else if (packet[0] & PS2Rightbutton)
    {
        ev.type = MOUSE_RIGHT_BUTTON;
    }
    else if (packet[0] & PS2Leftbutton)
    {
        ev.type = MOUSE_LEFT_BUTTON;
    }
    if (ev.type != MOUSE_NO_TYPE)
    {
        push_mouse_event(handler, ev);
        ev.type = MOUSE_NO_TYPE;
    }

    if (!x_neg)
    {
        ev.type = X_MOVEMENT;
        ev.value = packet[1];
        ev.value += x_overflow ? 255 : 0;
    }
    else
    {
        ev.type = X_MOVEMENT;
        ev.value = 256 - packet[1];
        ev.value *= -1;
        ev.value -= x_overflow ? 255 : 0;
    }
    if (ev.type != MOUSE_NO_TYPE)
    {
        push_mouse_event(handler, ev);
        ev.type = MOUSE_NO_TYPE;
    }
    if (!y_neg)
    {
        ev.type = Y_MOVEMENT;
        ev.value = packet[2];
        ev.value += y_overflow ? 255 : 0;
    }
    else
    {
        ev.type = Y_MOVEMENT;
        ev.value = 256 - packet[2];
        ev.value *= -1;
        ev.value -= y_overflow ? 255 : 0;
    }
    if (ev.type != MOUSE_NO_TYPE)
    {
        ev.value *= -1;
        push_mouse_event(handler, ev);
    }
}
void push_mouse_event(mouse_handler_t *handler, mouse_event_t ev)
{
    if (handler->mouse_stack_pos == 0)
    {
        handler->mouse_stack_pos = handler->buffer_size - 1;
    }
    handler->buffer[handler->mouse_stack_pos] = ev;
    handler->mouse_stack_pos--;
}
mouse_event_t pop_mouse_event(mouse_handler_t *handler)
{
    if (handler->mouse_stack_pos >= handler->buffer_size - 1)
    {
        return (mouse_event_t){};
    }
    handler->mouse_stack_pos++;
    mouse_event_t ev = handler->buffer[handler->mouse_stack_pos];
    return ev;
}