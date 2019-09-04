#include "gy-86.h"

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C总线设备句柄 */
#define I2C_BUS_NAME  "i2c1"

rt_err_t mpu6050_read_byte(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t *value)
{
		struct rt_i2c_msg msgs[2];
		rt_uint8_t tmp = reg;
	
		msgs[0].addr = MPU_ADDR;            /* 从机地址 */
    msgs[0].flags = RT_I2C_WR;     /* 读标志 */
    msgs[0].len = 1;
		msgs[0].buf = &tmp;

		msgs[1].addr  = MPU_ADDR;  /* Slave address */
    msgs[1].flags = RT_I2C_RD ;        /* Read flag */
    msgs[1].buf   = value;              /* Read data pointer */
    msgs[1].len   = 1;
		if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("from mpu6050 reg %x read byte error\n",reg);
        return -RT_ERROR;
    }
}
rt_err_t mpu6050_read_bytes(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t *buf, rt_uint8_t len)
{
		struct rt_i2c_msg msgs[2];
		rt_uint8_t tmp = reg;
	
		msgs[0].addr = MPU_ADDR;            /* 从机地址 */
    msgs[0].flags = RT_I2C_WR;     /* 读标志 */
    msgs[0].len = 1;
		msgs[0].buf = &tmp;

		msgs[1].addr  = MPU_ADDR;  /* Slave address */
    msgs[1].flags = RT_I2C_RD ;        /* Read flag */
    msgs[1].buf   = buf;              /* Read data pointer */
    msgs[1].len   = len;
		if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("from mpu6050 reg %x read bytes error\n",reg);
        return -RT_ERROR;
    }
}

rt_err_t mpu6050_init(void)
{
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT1_REG, 0X80);	//复位MPU6050
  rt_thread_mdelay(100);
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT1_REG,0X00);	//唤醒MPU6050 
	
	mpu6050_write_byte(i2c_bus, MPU_CFG_REG, 0x06);
	mpu6050_write_byte(i2c_bus, MPU_GYRO_CFG_REG, 0x18);//±2000dps
	mpu6050_write_byte(i2c_bus, MPU_ACCEL_CFG_REG, 0x00);//±2g
	
	//下面需要根据自己的需求设置寄存器
	//不设置低通滤波器，以获得最快的数据更新。
	//其实输出速率快，采样率块应该不重要。重要的应该是带宽。260Hz
	rt_uint16_t samplae_rate = 8000;
	mpu6050_write_byte(i2c_bus, MPU_SAMPLE_RATE_REG, 8000/samplae_rate -1);
	mpu6050_write_byte(i2c_bus, MPU_CFG_REG, 0);//不设置低通滤波
	//其实手册的那个表里面，MPU_CFG_REG，设置为0的话，陀螺仪是8kHz，加速度是1kHz的采样率，更高，这样就是不使用低通滤波器。
	//看网上的一些说法，这个低通滤波器是滤掉高频噪声，那实际角度变化的频率肯定不能超过这个频率（超过了就不能用mpu6050了），但是设置了
	//5.5ms更新数据有点低，不设置的话就是3.8ms（其实也好不到哪里去）
	//所以后续的pid控制周期要是小于3.8ms就没意义了。
	
	rt_uint8_t read_value = 0;
	mpu6050_read_byte(i2c_bus, MPU_DEVICE_ID_REG, &read_value);
	
	if(read_value != 0x68)
	{
		return -RT_ERROR;
	}
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT1_REG, 0X01);
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT2_REG, 0X00);
	
	mpu6050_write_byte(i2c_bus, MPU_SAMPLE_RATE_REG, 8000/samplae_rate -1);
	mpu6050_write_byte(i2c_bus, MPU_CFG_REG, 0);
	return RT_EOK;
}
void gy86_init(void)
{
		i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C_BUS_NAME);
		if (i2c_bus == RT_NULL)
    {
        rt_kprintf("\ncan't find %s device!\n", I2C_BUS_NAME);
				return;
    }
		/*init MPU6050*/
		if(mpu6050_init() == -RT_ERROR)
		{
				rt_kprintf("\nmpu6050 init error\n");
				return;
		}
		rt_kprintf("\nmpu6050 init success\n");
		
		/*init HMC5883L*/
		if(hmc5883l_init() == -RT_ERROR)
		{
				rt_kprintf("\nhmc5883l_init init error\n");
				return;
		}
		rt_kprintf("\nhmc5883l_init init success\n");
		return ;
}
rt_err_t mpu6050_get_accelerometer(struct imu_data* imu)
{
	rt_uint8_t buf[6];
	if(mpu6050_read_bytes(i2c_bus, MPU_ACCEL_XOUTH_REG, buf, 6) != RT_EOK)return -RT_ERROR;
	imu->ax=((rt_uint16_t)buf[0]<<8)|buf[1];  
	imu->ay=((rt_uint16_t)buf[2]<<8)|buf[3];  
	imu->az=((rt_uint16_t)buf[4]<<8)|buf[5];
	return RT_EOK;
}
rt_err_t mpu6050_get_gyroscope(struct imu_data* imu)
{
	rt_uint8_t buf[6];
	if(mpu6050_read_bytes(i2c_bus, MPU_GYRO_XOUTH_REG, buf, 6) != RT_EOK)return -RT_ERROR;
	imu->gx=((rt_uint16_t)buf[0]<<8)|buf[1];  
	imu->gy=((rt_uint16_t)buf[2]<<8)|buf[3];  
	imu->gz=((rt_uint16_t)buf[4]<<8)|buf[5];
	return RT_EOK;
}
rt_err_t mpu6050_write_byte(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t value)
{
		struct rt_i2c_msg msgs;
		rt_uint8_t buf[2];
		msgs.addr = MPU_ADDR;            /* 从机地址 */
    msgs.flags = RT_I2C_WR;     /* 写标志 */
		buf[0] = reg;
		buf[1] = value;
    msgs.len = 2;
		msgs.buf = buf;
		if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("mpu6050 write reg &x error\n",reg);
        return -RT_ERROR;
    }
}
rt_err_t hmc5883l_write_byte(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t value)
{
		struct rt_i2c_msg msgs;
		rt_uint8_t buf[2];
		msgs.addr = HMC5883L_Addr;            /* 从机地址 */
    msgs.flags = RT_I2C_WR;     /* 写标志 */
		buf[0] = reg;
		buf[1] = value;
    msgs.len = 2;
		msgs.buf = buf;
		if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("hmc5883l write reg %x error\n",reg);
        return -RT_ERROR;
    }
}
rt_err_t hmc5883l_read_bytes(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t *buf, rt_uint8_t len)
{
		struct rt_i2c_msg msgs[2];
		rt_uint8_t tmp = reg;
	
		msgs[0].addr = HMC5883L_Addr;            /* 从机地址 */
    msgs[0].flags = RT_I2C_WR;     /* 读标志 */
    msgs[0].len = 1;
		msgs[0].buf = &tmp;

		msgs[1].addr  = HMC5883L_Addr;  /* Slave address */
    msgs[1].flags = RT_I2C_RD ;        /* Read flag */
    msgs[1].buf   = buf;              /* Read data pointer */
    msgs[1].len   = len;
		if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("from hmc5883l reg %x read bytes error\n",reg);
        return -RT_ERROR;
    }
}

rt_err_t hmc5883l_init(void)
{
	mpu6050_write_byte(i2c_bus, MPU_USER_CTRL_REG,0X00);//close Master Mode 
	mpu6050_write_byte(i2c_bus, MPU_INTBP_CFG_REG,0X02);//turn on Bypass Mode 
	
	hmc5883l_write_byte(i2c_bus,HMC58X3_R_CONFA,0x78);   
	//75Hz输出，看手册如果设置0x78就是8个数平均，滤波了，不一定好
	
  hmc5883l_write_byte(i2c_bus,HMC58X3_R_CONFB,0x00);   //测量范围+—4.7Ga	390 counts/高斯
  hmc5883l_write_byte(i2c_bus,HMC58X3_R_MODE,0x00);    //
	return RT_EOK;
}
rt_err_t hmc5883l_get_magnetic(struct imu_data* imu)
{
	rt_uint8_t buf[6] = {0};
	
	hmc5883l_read_bytes(i2c_bus, HMC5883L_RA_DATA_OUTPUT_X_MSB, buf, 6);

	imu->mx=(buf[0] << 8) | buf[1]; //Combine MSB and LSB of X Data output register
	imu->mz=(buf[2] << 8) | buf[3]; //Combine MSB and LSB of Y Data output register
	imu->my=(buf[4] << 8) | buf[5]; //Combine MSB and LSB of Z Data output register
	
	if(imu->mx>0x7fff)imu->mx-=0xffff;	  
	if(imu->my>0x7fff)imu->my-=0xffff;
	if(imu->mz>0x7fff)imu->mz-=0xffff;
	return RT_EOK;
}
