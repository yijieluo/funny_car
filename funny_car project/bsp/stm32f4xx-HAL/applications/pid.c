#include "pid.h"
#include "stm32f4xx.h"

#define ABS(x)		((x>0)? x: -x) 

//PID_TypeDef pid_pitch,pid_pithch_speed,pid_roll,pid_roll_speed,pid_yaw_speed;

/*参数初始化--------------------------------------------------------------*/
static void pid_param_init(
	PID_TypeDef * pid, 
	PID_ID   id,
	float maxout,
	float intergral_limit,
	float deadband,
	float period,
	float  max_err,
	float  target,

	float 	kp, 
	float 	ki, 
	float 	kd)
{
	pid->id = id;		
	
	pid->ControlPeriod = period;             //没用到
	pid->DeadBand = deadband;
	pid->IntegralLimit = intergral_limit;
	pid->MaxOutput = maxout;
	pid->Max_Err = max_err;
	pid->target = target;
	
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
	
	pid->output = 0;
}

/*中途更改参数设定--------------------------------------------------------------*/
static void pid_reset(PID_TypeDef * pid, float kp, float ki, float kd)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
}

/*pid计算-----------------------------------------------------------------------*/
static float pid_calculate(PID_TypeDef* pid, float measure, float target)
{
	pid->measure = measure;
 	pid->target = target;
	pid->last_err  = pid->err;
	pid->last_output = pid->output;
	
	pid->err = pid->target - pid->measure;
	
	//是否进入死区
	if((ABS(pid->err) > pid->DeadBand))
	{
		pid->pout = pid->kp * pid->err;
		pid->iout += (pid->ki * pid->err);
		pid->dout =  pid->kd * (pid->err - pid->last_err); 
		
		//积分是否超出限制
		if(pid->iout > pid->IntegralLimit)
			pid->iout = pid->IntegralLimit;
		if(pid->iout < - pid->IntegralLimit)
			pid->iout = - pid->IntegralLimit;
		
		//pid输出和
		pid->output = pid->pout + pid->iout + pid->dout;
		
		if(pid->output>pid->MaxOutput)         
		{
			pid->output = pid->MaxOutput;
		}
		if(pid->output < -(pid->MaxOutput))
		{
			pid->output = -(pid->MaxOutput);
		}
	}
	return pid->output;
}

PID_TypeDef motor_pid[6];//左轮速度环，右轮速度环，左轮角度环，右轮角度环 左轮转向环 右轮转向环
void pid_init()
{
	//left wheel pid init
	motor_pid[0].f_param_init = pid_param_init;
	motor_pid[0].f_pid_reset = pid_reset;
	motor_pid[0].f_cal_pid = pid_calculate;	
	motor_pid[0].f_param_init(&motor_pid[0],PID_Speed,10000,9000,10,0,8000,0,7.5f,0.2f,6.2f);
	
	//right wheel pid init
	motor_pid[1].f_param_init = pid_param_init;
	motor_pid[1].f_pid_reset = pid_reset;
	motor_pid[1].f_cal_pid = pid_calculate;
	motor_pid[1].f_param_init(&motor_pid[1],PID_Speed,10000,9000,10,0,8000,0,7.5f,0.2f,6.2f);
	
	motor_pid[2].f_param_init = pid_param_init;
	motor_pid[2].f_pid_reset = pid_reset;
	motor_pid[2].f_cal_pid = pid_calculate;
	motor_pid[2].f_param_init(&motor_pid[2],PID_Position,7000,7000,0,0,60,0,560,70,130);
	
	motor_pid[3].f_param_init = pid_param_init;
	motor_pid[3].f_pid_reset = pid_reset;
	motor_pid[3].f_cal_pid = pid_calculate;
	motor_pid[3].f_param_init(&motor_pid[3],PID_Position,7000,7000,0,0,60,0,560,70,130);
}
//struct _PID_TypeDef *pid,PID_ID id,uint16_t maxOutput,uint16_t integralLimit,float deadband,uint16_t controlPeriod,int16_t max_err,int16_t  target,
//				   float kp,
//				   float ki,
//				   float kd
