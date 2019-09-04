#include <rtthread.h>
#include <board.h>
#include "can_app.h"
#include "timer_app.h"
#include "gy-86.h"
#include "imu.h"
#include "pid.h"
#include "motor.h"
#include "key.h"
#include "led.h"
#include "ps2.h"

struct imu_data imu;
rt_device_t can_dev = RT_NULL;
rt_device_t uart2_dev = RT_NULL;
rt_timer_t timer1;
float pitch,roll,yaw;
float roll_offset = 0;
float roll_target;
float rotation;
extern moto_measure_t moto_chassis[2];

int main(void)
{
		/*can设备初始化*/
		can_dev = rt_device_find(CAN1_DEVICE_NAME);
		if (!can_dev)
		{
				rt_kprintf("find %s failed!\n", CAN1_DEVICE_NAME);
				return RT_ERROR;
		}
		rt_device_open(can_dev, (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX));
		if(rt_device_control(can_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN1MBaud) != RT_EOK)
		{
				rt_kprintf("config %s failed!\n", CAN1_DEVICE_NAME);
		}
		rt_device_set_rx_indicate(can_dev, can1_receive);//设置接收回调函数
		
		//ps2遥控初始化
		ps2_init();
		
		//左右轮pid参数初始化
		pid_init();
		
		set_moto_current(can_dev,0,0);
		rt_thread_mdelay(1000);
		
		/*传感器GY-86初始化*/
		gy86_init();
		
		mpu6050_get_gyroscope(&imu);
		mpu6050_get_accelerometer(&imu);
		hmc5883l_get_magnetic(&imu);
		
		MahonyAHRSupdate(imu.gx/2000.0,imu.gy/2000.0,imu.gz/2000.0,imu.ax,imu.ay,imu.az,imu.mx,imu.my,imu.mz,&pitch,&roll,&yaw);
		
		roll_offset = roll;
		
		/*串口2初始化 蓝牙*/
		uart2_dev = rt_device_find("uart2");
    if (!uart2_dev)
    {
        rt_kprintf("find %s failed!\n", "uart2");
        return RT_ERROR;
    }
		rt_device_open(uart2_dev, RT_DEVICE_FLAG_INT_RX);	
			
		/* 软定时器初始化 */
    /* 创建定时器 1  周期定时器 */
    timer1 = rt_timer_create("timer1", balance_control,
                             RT_NULL, 1,
                             RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER);

    /* 启动定时器 1 */
    if (timer1 != RT_NULL) rt_timer_start(timer1);
		
		/* 按键线程 */
		key_init();
		
		/* LED状态显示线程 */
		led_init();
		
		/* 电机电流保护线程 */ //pcb设计电流只有8A左右，电机全速电流是10A。
		motor_init();
		
		//因为这些线程初始化，顺序需要注意一下，用EXPORT宏不知道如何处理顺序

		/*main线程 一直更新imu数据*/
		while(1)
		{
			mpu6050_get_gyroscope(&imu);
			mpu6050_get_accelerometer(&imu);
			hmc5883l_get_magnetic(&imu);
			rt_thread_delay(1);
		}
}
//	float waveform[3] = {0};
//	waveform[0] = -rmp_target;
//	waveform[1] = moto_chassis[0].speed_rpm;
//	waveform[2] = moto_chassis[1].speed_rpm;
//	send_waveform_fomate(waveform, sizeof(waveform));	

//		/*硬件定时器初始化*/
//		hw_dev = rt_device_find(HWTIMER_DEV_NAME);
//		if (hw_dev == RT_NULL)
//    {
//        rt_kprintf("can't find %s device!\n", HWTIMER_DEV_NAME);
//        //return RT_ERROR;
//    }
//		ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
//		if (ret != RT_EOK)
//    {
//        rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
//        return ret;
//    }
//		if(rt_device_set_rx_indicate(hw_dev, balance_control) != RT_EOK)
//		{
//				rt_kprintf("set_rx_indicate failed!\n");
//		}
//		rt_hwtimer_mode_t mode = HWTIMER_MODE_PERIOD;
//    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
//    if (ret != RT_EOK)
//    {
//        rt_kprintf("set mode failed! ret is :%d\n", ret);
//        return ret;
//    }
//		rt_hwtimerval_t timeout_s = {0,1000};//1ms
//		if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
//    {
//        rt_kprintf("set timeout value failed\n");
//        return RT_ERROR;
//    }
