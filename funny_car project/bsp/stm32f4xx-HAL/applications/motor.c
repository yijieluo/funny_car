#include "motor.h"
#include "can_app.h"

#define THREAD_PRIORITY             16
#define THREAD_STACK_SIZE           256
#define THREAD_TIMESLICE            1

#define ABS(x)		((x>0)? x: -x) 
static rt_thread_t      tid_motor      =   RT_NULL;


extern moto_measure_t moto_chassis[2];
extern rt_device_t can_dev;
extern rt_timer_t timer1;

#define SPEED_LIMIT 6800
static void motor_thread_entry(void *param) //电机电流保护线程
{
		while(1)
		{
				if(ABS(moto_chassis[0].speed_rpm) > SPEED_LIMIT || ABS(moto_chassis[1].speed_rpm) > SPEED_LIMIT)
				{
						rt_thread_mdelay(1000);
						if(ABS(moto_chassis[0].speed_rpm) > SPEED_LIMIT || ABS(moto_chassis[1].speed_rpm) > SPEED_LIMIT)
						{
								set_moto_current(can_dev, 0, 0);
								rt_timer_stop(timer1);
								break;
						}
				}
				rt_thread_mdelay(100);
		}
}

int motor_init(void)
{
			tid_motor = rt_thread_create("motor",
                          motor_thread_entry, RT_NULL,
                          THREAD_STACK_SIZE,
                          THREAD_PRIORITY, THREAD_TIMESLICE);

			if (tid_motor != RT_NULL)
			{
					rt_thread_startup(tid_motor);
			}
			else
			{
					rt_kprintf("Can't create thread for motor");
					return -RT_ENOSYS;
			} 
			return RT_EOK;
}
