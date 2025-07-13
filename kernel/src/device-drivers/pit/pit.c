#include "pit.h"
#include "../../io/io.h"
#include "../../kernel-trace/kernel_trace.h"

double time_since_boot = 0;
uint16_t pit_divisor = 65535;
const uint64_t base_frequency = 1193182;

void sleep(uint64_t ms); // in milliseconds

void pit_set_divisor(uint16_t divisor)
{
    KERN_TRACE_FUNC;
    divisor = (divisor < 100) ? 100 : divisor;
    pit_divisor = divisor;
    outb(0x40, (uint8_t)(divisor & 0x00FF));
    io_wait();
    outb(0x40, (uint8_t)((divisor & 0xFF00) >> 8));
}

uint64_t pit_get_frequency()
{
    return base_frequency / pit_divisor;
}

void pit_set_frequency(uint64_t frequency)
{
    KERN_TRACE_FUNC;
    pit_set_divisor(base_frequency / frequency);
}

void pit_timer_tick()
{
    time_since_boot += 1.0f / (double)pit_get_frequency();
}

void sleep(uint64_t ms)
{
    double start_time = time_since_boot;
    double end_time = start_time + ((double)ms / 1000.0f);
    while (time_since_boot < end_time)
        ;
}