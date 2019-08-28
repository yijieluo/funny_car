#include <can_app.h>
#include "filter.h"

moto_measure_t moto_chassis[2] = {0};
extern struct list* header;
extern rt_uint8_t list_len;

static int16_t rpm_buf[2];
void get_moto_measure(moto_measure_t *ptr, struct rt_can_msg* prmsg)
{

	ptr->last_angle = ptr->angle;
	ptr->angle = (uint16_t)(prmsg->data[0]<<8 | prmsg->data[1]) ;
	ptr->speed_rpm  = (int16_t)(prmsg->data[2]<<8 | prmsg->data[3]);
	
	if(prmsg->id == CAN_2006Moto1_ID)
	{
			rpm_buf[0] = ptr->speed_rpm;
	}
	else if(prmsg->id == CAN_2006Moto2_ID)
	{
			rpm_buf[1] = ptr->speed_rpm;
	}
	else
		;
	//滑动窗口滤波
	if(list_len == 0)
		list_creat(rpm_buf);
	else
		list_push(rpm_buf);
	
	list_avr(rpm_buf);
	if(prmsg->id == CAN_2006Moto1_ID)
	{
			ptr->speed_rpm = rpm_buf[0];
	}
	else if(prmsg->id == CAN_2006Moto2_ID)
	{
			ptr->speed_rpm = rpm_buf[1];
	}
	//ptr->real_current = (prmsg->data[4]<<8 | prmsg->data[5])*5.f/16384.f;

	//ptr->hall = prmsg->data[6];
	
	if(ptr->angle - ptr->last_angle > 4096)
		ptr->round_cnt --;
	else if (ptr->angle - ptr->last_angle < -4096)
		ptr->round_cnt ++;
	ptr->total_angle = ptr->round_cnt * 8192 + ptr->angle - ptr->offset_angle;
}

struct rt_can_msg r_msg;
rt_err_t can1_receive(rt_device_t dev, rt_size_t size)
{ 
  rt_size_t test_data;//测试查看用
	test_data=rt_device_read(dev,0,&r_msg,sizeof(r_msg));
	
	switch(r_msg.id)
	{
		case CAN_2006Moto1_ID:
		case CAN_2006Moto2_ID:
//		case CAN_2006Moto3_ID:
//		case CAN_2006Moto4_ID:
			{
				static unsigned char i;
				i = r_msg.id - CAN_2006Moto1_ID;
				
				get_moto_measure(&moto_chassis[i], &r_msg);
			}
			break;
	}
	if(test_data==0)
  {
    rt_kprintf("read failed!\n");
  }
  return RT_EOK;
}

struct rt_can_msg t_msg;
#define LIMIT_SPEED 8000
void set_moto_current(rt_device_t can_dev,short left_value, short right_value)
{
	if(left_value > LIMIT_SPEED) left_value = LIMIT_SPEED;
	if(left_value < -LIMIT_SPEED) left_value = -LIMIT_SPEED;
	
	if(right_value > LIMIT_SPEED) right_value = LIMIT_SPEED;
	if(right_value < -LIMIT_SPEED) right_value = -LIMIT_SPEED;
	
	t_msg.ide=CAN_ID_STD;
	t_msg.rtr=CAN_RTR_DATA;
	t_msg.len=8;
	t_msg.id=0x200;
	t_msg.data[0] = (left_value >> 8);
	t_msg.data[1] = left_value;
	t_msg.data[2] = (right_value >> 8);
	t_msg.data[3] = right_value;
	
	t_msg.data[4] = 0;
	t_msg.data[5] = 0;
	t_msg.data[6] = 0;
	t_msg.data[7] = 0;
	
	rt_device_write(can_dev, 0, &t_msg, sizeof(t_msg));
}
