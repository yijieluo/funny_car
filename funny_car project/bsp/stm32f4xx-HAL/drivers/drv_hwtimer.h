#ifndef _DRV_HWTIMER_H_
#define _DRV_HWTIMER_H_

#include <rtthread.h>
#include <rtdevice.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef TIM_DEV_INFO_CONFIG
#define TIM_DEV_INFO_CONFIG                     \
    {                                           \
        .maxfreq = 1000000,                     \
        .minfreq = 3000,                        \
        .maxcnt  = 0xFFFF,                      \
        .cntmode = HWTIMER_CNTMODE_UP,          \
    }
#endif /* TIM_DEV_INFO_CONFIG */

#ifdef BSP_USING_TIM2
#ifndef TIM2_CONFIG
#define TIM2_CONFIG                                        \
    {                                                      \
       .tim_handle.Instance     = TIM2,                    \
       .tim_irqn                = TIM2_IRQn,               \
       .name                    = "timer2",                \
    }
#endif /* TIM6_CONFIG */
#endif /* BSP_USING_TIM2 */



#ifdef __cplusplus
}
#endif

int rt_hw_hwtimer_init(void);

#endif
