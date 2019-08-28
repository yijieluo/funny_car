#ifndef _CAN_APP_H_
#define _CAN_APP_H_

#include <rtthread.h>
#include <rtdevice.h>

#define CAN_ID_STD                  0x00000000U  /*!< Standard Id */
#define CAN_ID_EXT                  0x00000004U  /*!< Extended Id */

#define CAN_RTR_DATA                0x00000000U  /*!< Data frame */
#define CAN_RTR_REMOTE              0x00000002U  /*!< Remote frame */

#define CAN1_DEVICE_NAME "can1"

typedef enum
{

	CAN_2006Moto_ALL_ID = 0x200,
	CAN_2006Moto1_ID = 0x201,
	CAN_2006Moto2_ID = 0x202,
	CAN_2006Moto3_ID = 0x203,
	CAN_2006Moto4_ID = 0x204,
	
}CAN_Message_ID;

#define FILTER_BUF_LEN		5
/*接收到的云台电机的参数结构体*/
typedef struct{
	int16_t	 	speed_rpm;
  float  	real_current;
  int16_t  	given_current;
  uint8_t  	hall;
	uint16_t 	angle;				//abs angle range:[0,8191]
	uint16_t 	last_angle;	//abs angle range:[0,8191]
	uint16_t	offset_angle;
	int32_t		round_cnt;
	int32_t		total_angle;
	unsigned char			buf_idx;
	unsigned short		angle_buf[FILTER_BUF_LEN];
	unsigned short		fited_angle;
	unsigned int		  msg_cnt;
}moto_measure_t;


rt_err_t can1_receive(rt_device_t dev, rt_size_t size);
void set_moto_current(rt_device_t can_dev,short left_value, short right_value);
#endif
