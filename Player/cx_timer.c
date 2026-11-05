#include "cx_timer.h"

#define REG32(address) (*(volatile uint32_t *)(address))
#define TIMER(offset) REG32(0x900D0000u + (offset))
#define VIC(offset) REG32(0xDC000000u + (offset))
#define TIMER_IRQ (1u << 19)
#define PERIODIC_CONTROL 0x62u /* periodic | interrupt enable | 32-bit */
#define ENABLE 0x80u

static uint32_t saved_control, saved_load, saved_value, saved_clock;
static int saved;

void cx_timer_restore(void)
{
    if (!saved)
        return;
    TIMER(0x08) = 0;
    cx_timer_ack();
    TIMER(0x80) = saved_clock;
    TIMER(0x00) = saved_value;
    TIMER(0x18) = saved_load;
    TIMER(0x08) = saved_control;
    saved = 0;
}

int cx_timer_init(uint32_t reload)
{
    if (!reload)
        return -1;

    if (saved)
        return -1;
    saved_control = TIMER(0x08);
    saved_load = TIMER(0x00);
    saved_value = TIMER(0x04);
    saved_clock = TIMER(0x80);
    saved = 1;
    TIMER(0x08) = 0;
    TIMER(0x80) = 2; /* CX-specific clock selector: approximately 32 kHz. */
    cx_timer_ack();
    TIMER(0x00) = reload;
    TIMER(0x08) = PERIODIC_CONTROL;
    return 0;
}

void cx_timer_start(void)
{
    TIMER(0x08) |= ENABLE;
}

void cx_timer_stop(void)
{
    TIMER(0x08) &= ~ENABLE;
}

int cx_timer_set_period(uint32_t reload)
{
    if (!saved || !reload)
        return -1;
    uint32_t control = TIMER(0x08);
    TIMER(0x08) = control & ~ENABLE;
    cx_timer_ack();
    TIMER(0x00) = reload;
    TIMER(0x08) = control;
    return 0;
}

uint32_t cx_timer_value(void)
{
    return TIMER(0x04);
}

int cx_timer_pending(void)
{
    return (TIMER(0x10) & 1u) != 0; /* Raw status, also usable by polling. */
}

void cx_timer_ack(void)
{
    TIMER(0x0C) = 1;
}

void cx_timer_irq_enable(void)
{
    VIC(0x0C) &= ~TIMER_IRQ;
    VIC(0x10) = TIMER_IRQ;
}

void cx_timer_irq_disable(void)
{
    VIC(0x14) = TIMER_IRQ;
}

void cx_timer_irq_begin(void)
{
    (void)VIC(0x30);
}

void cx_timer_irq_end(void)
{
    VIC(0x30) = 0;
}
