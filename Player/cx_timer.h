#ifndef CX_TIMER_H
#define CX_TIMER_H

#include <stdint.h>

int cx_timer_init(uint32_t reload);
/* Restore configuration saved by init; elapsed/pending OS ticks are not replayed. */
void cx_timer_restore(void);
void cx_timer_start(void);
void cx_timer_stop(void);
uint32_t cx_timer_value(void);
int cx_timer_pending(void);
void cx_timer_ack(void);

void cx_timer_irq_enable(void);
void cx_timer_irq_disable(void);

void cx_timer_irq_begin(void);
void cx_timer_irq_end(void);

#endif
