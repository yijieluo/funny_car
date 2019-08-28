#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

static rt_uint32_t tick6=0;
static rt_uint32_t tick7=0;
rt_err_t timer6_callback(rt_device_t dev, rt_size_t size);
rt_err_t timer7_callback(rt_device_t dev, rt_size_t size);
static struct rt_semaphore period6sem;
static struct rt_semaphore period7sem;

void rt_period_thread_entry(void *parameter)
{
	rt_device_t rt_d;
	rt_hwtimer_t *hw_dev;
	rt_uint32_t p = 10000;
	rt_d = rt_device_find("timer6");

	if(rt_d != RT_NULL)
	{
		hw_dev = (rt_hwtimer_t *)rt_d;
		hw_dev->parent.rx_indicate = &timer6_callback;
		hw_dev->ops->init(hw_dev,1);
		hw_dev->ops->control(hw_dev,HWTIMER_CTRL_FREQ_SET,&p);
		hw_dev->ops->start(hw_dev,10000-1,HWTIMER_MODE_PERIOD);
	}
	
	rt_d = rt_device_find("timer7");
	
	if(rt_d != RT_NULL)
	{
		hw_dev = (rt_hwtimer_t *)rt_d;
		hw_dev->parent.rx_indicate = &timer7_callback;
		hw_dev->ops->init(hw_dev,1);
		hw_dev->ops->control(hw_dev,HWTIMER_CTRL_FREQ_SET,&p);
		hw_dev->ops->start(hw_dev,2000-1,HWTIMER_MODE_PERIOD);
	}
	
	while(1)
	{
		rt_sem_take(&period7sem, RT_WAITING_FOREVER);
		rt_sem_take(&period6sem, RT_WAITING_FOREVER);
		
		//rt_kprintf("tick:%d \r\n",tick);
	}
}

rt_err_t timer7_callback(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&period7sem);
	tick7++;
	return RT_EOK;
}

rt_err_t timer6_callback(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&period6sem);
	tick6++;
	return RT_EOK;
}

int rt_period_thread_init(void)
{
	rt_thread_t period_thread;
	period_thread = rt_thread_create("period6",rt_period_thread_entry,RT_NULL,1024,7,20);
	rt_sem_init(&period6sem, "period6sem", 0, RT_IPC_FLAG_FIFO);
	rt_sem_init(&period7sem, "period7sem", 0, RT_IPC_FLAG_FIFO);
	
	if(period_thread != RT_NULL)
	{
		rt_thread_startup(period_thread);
	}

	return 0;
}
INIT_APP_EXPORT(rt_period_thread_init);

static int tick_get(int argc, char **argv)
{
	rt_kprintf("tick6:%d \r\n",tick6);
	rt_kprintf("tick7:%d \r\n",tick7);
	return 0;
}
MSH_CMD_EXPORT(tick_get,tick_get);
