#ifndef _IMU_H_
#define _IMU_H_
#include <rtthread.h>
#include <rtdevice.h>
#include <math.h>

void MahonyAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float* pitch, float* roll, float* yaw);
//void MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az, float* pitch, float* roll, float* yaw);
#endif
