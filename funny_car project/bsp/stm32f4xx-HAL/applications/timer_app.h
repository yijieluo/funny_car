#ifndef _TIMER_APP_H_
#define _TIMER_APP_H_

#include <rtthread.h>
#include <rtdevice.h>
#include "string.h"

#define SOFT_TIMER_DEV_NAME "timer1"

//#define HWTIMER_DEV_NAME "timer2"

void balance_control(void* parameter);
rt_err_t send_waveform_fomate(void *buf, uint32_t size);

#endif
