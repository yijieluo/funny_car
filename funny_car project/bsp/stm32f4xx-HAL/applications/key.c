#include "key.h"
#include "timer_app.h"
#include "can_app.h"

extern rt_device_t can_dev;
extern rt_timer_t timer1;

#define THREAD_PRIORITY             14
#define THREAD_STACK_SIZE           256
#define THREAD_TIMESLICE            5

static rt_thread_t      tid_key      =   RT_NULL;

rt_uint8_t key_scan()
{
		if(rt_pin_read(KEY_PIN) == 0)//如果按键按下
		{
				rt_thread_mdelay(20);//消抖
				if(rt_pin_read(KEY_PIN) == 0)//确认按键按下
				{
						return 1;
				}
		}
		return 0;
}
static void key_thread_entry(void *param)
{   
    while (1)
    {
				if(key_scan() == 1)
				{
						set_moto_current(can_dev,0,0);
						rt_timer_stop(timer1);
						break;
				}
        rt_thread_mdelay(10);   
    }
}
int key_init()
{
		rt_pin_mode(KEY_PIN, PIN_MODE_INPUT_PULLUP);
	
		tid_key = rt_thread_create("key",
												key_thread_entry, RT_NULL,
												THREAD_STACK_SIZE,
												THREAD_PRIORITY, THREAD_TIMESLICE);

    if (tid_key != RT_NULL)
    {
        rt_thread_startup(tid_key);
    }
    else
    {
        rt_kprintf("Can't create thread for key");
        return RT_ERROR;
    }   
    return RT_EOK;
}
