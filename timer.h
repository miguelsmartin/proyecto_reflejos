/*
 * timer.h
 *
 *  Created on: 3 de may. de 2016
 *      Author: miguelsanchez
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <signal.h>
#include <time.h>
struct tmr_t {
    timer_t timerid;
    struct itimerspec spec;
    struct sigevent se;
};
typedef struct tmr_t tmr_t;
typedef void (*notify_func_t) (union sigval);
tmr_t* tmr_new (notify_func_t isr);
void tmr_init (tmr_t* this, notify_func_t isr);
void tmr_destroy(tmr_t* this);
void tmr_startms(tmr_t* this, int ms);
void tmr_stop (tmr_t* this);

#endif /* TIMER_H_ */
