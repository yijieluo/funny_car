#include "timer_app.h"
#include "can_app.h"
#include "gy-86.h"
#include "imu.h"
#include "pid.h"
#include <rthw.h>

extern float pitch,roll,yaw;
extern struct imu_data imu;
extern rt_device_t can_dev;
extern rt_device_t uart2_dev;
extern PID_TypeDef motor_pid[6];
extern moto_measure_t moto_chassis[2];
float rmp_target;
extern float roll_offset;
extern float roll_target;
extern float rotation;

void balance_control(void* parameter)
{
		rt_base_t level0;
//		level0 = rt_hw_interrupt_disable();
		static rt_uint16_t cnt = 0;
		cnt++;
		if(cnt%2==0)
		{
				motor_pid[0].f_cal_pid(&motor_pid[0], moto_chassis[0].speed_rpm,  motor_pid[2].output+rotation);
				motor_pid[1].f_cal_pid(&motor_pid[1], moto_chassis[1].speed_rpm, -motor_pid[3].output+rotation);
				
				level0 = rt_hw_interrupt_disable();//can传输不能被中断打断，不知道为啥。
			  set_moto_current(can_dev, 
											  (short)motor_pid[0].output,
												(short)motor_pid[1].output);								
				rt_hw_interrupt_enable(level0);
		}
		if(cnt%5 == 0)
		{
				MahonyAHRSupdate(imu.gx/2000.0f,imu.gy/2000.0f,imu.gz/2000.0f,imu.ax,imu.ay,imu.az,imu.mx,imu.my,imu.mz,&pitch,&roll,&yaw);
				motor_pid[2].f_cal_pid(&motor_pid[2], roll, roll_target+roll_offset);
				motor_pid[3].f_cal_pid(&motor_pid[3], roll, roll_target+roll_offset);
			
				/* 由于是遥控，转向环不需要闭环*/
//				motor_pid[4].f_cal_pid(&motor_pid[4], );
//				motor_pid[5].f_cal_pid(&motor_pid[5], );
				cnt = 0;
		}
}
/*串口2 接蓝牙，配合山外调试助手看波形*/
rt_err_t send_waveform_fomate(void *buf, uint32_t size)
{
    const char start[2] = {0x03, 0xfc};
    const char end[2]   = {0xfc, 0x03};

    rt_device_write(uart2_dev, 0, start, 2);
    rt_device_write(uart2_dev, 0, buf, size);
    rt_device_write(uart2_dev, 0, end, 2);

    return RT_EOK;
}

/*在线修改pid参数*/
static void cmd(int argc, char**argv)
{
		if(rt_strcmp(argv[0], "cmd") || argc != 2)
		{
				rt_kprintf("cmd error\n");
        return;
		}
    if (!rt_strcmp(argv[1], "kp+=10"))
    {
				motor_pid[2].kp += 10;
				motor_pid[3].kp += 10;
        rt_kprintf("kp+=10 success\n");
    }
		else if (!rt_strcmp(argv[1], "kp-=10"))
    {
				motor_pid[2].kp -= 10;
				motor_pid[3].kp -= 10;
        rt_kprintf("kp-=10 success\n");
    }
		else if (!rt_strcmp(argv[1], "ki+=1"))
    {
				motor_pid[2].ki += 1;
				motor_pid[3].ki += 1;
        rt_kprintf("ki+= 1 success\n");
    }
		else if (!rt_strcmp(argv[1], "ki-=1"))
    {
				motor_pid[2].ki -= 1;
				motor_pid[3].ki -= 1;
        rt_kprintf("ki-=1 success\n");
    }
		else if (!rt_strcmp(argv[1], "kd+=5"))
    {
				motor_pid[2].kd += 5;
				motor_pid[3].kd += 5;
        rt_kprintf("kd+=5 success\n");
    }
		else if (!rt_strcmp(argv[1], "kd-=5"))
    {
				motor_pid[2].kd -= 5;
				motor_pid[3].kd -= 5;
        rt_kprintf("kd-=5 success\n");
    }
		else if (!rt_strcmp(argv[1], "list_param"))
    {
        rt_kprintf("kp*100 = %d, ki*100 = %d, kd*100 = %d\n", (int)(motor_pid[2].kp*100), (int)(motor_pid[2].ki*100), (int)(motor_pid[2].kd*100));
    }
		else
		{
				rt_kprintf("cmd error\n");
		}
}

MSH_CMD_EXPORT(cmd, cmd sample: cmd kp+=10);
