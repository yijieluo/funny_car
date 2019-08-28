#include "led.h"

#define THREAD_PRIORITY             15
#define THREAD_STACK_SIZE           256
#define THREAD_TIMESLICE            2

static rt_thread_t      tid_led      =   RT_NULL;

rt_uint8_t led_state = 0;

/* led状态显示线程 */
static void led_thread_entry(void *param)
{
		while(1)
		{
				if(led_state == 0) //正常状态
				{
						rt_pin_write(LED0_PIN, !rt_pin_read(LED0_PIN));
						rt_thread_mdelay(500);
				}
				else if(led_state == 1) //其它状态，未使用到
				{
					rt_pin_write(LED0_PIN, PIN_HIGH);
					rt_thread_mdelay(200);
					rt_pin_write(LED0_PIN, PIN_LOW);
					rt_thread_mdelay(200);
				}
				else
					;
		}
}

int led_init(void)
{
		rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
	
		tid_led = rt_thread_create("led",
                          led_thread_entry, RT_NULL,
                          THREAD_STACK_SIZE,
                          THREAD_PRIORITY, THREAD_TIMESLICE);

    if (tid_led != RT_NULL)
    {
        rt_thread_startup(tid_led);
    }
    else
    {
        rt_kprintf("Can't create thread for led");
        return -RT_ENOSYS;
    } 
    return RT_EOK;
}
