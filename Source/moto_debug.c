/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               motor_debug.h
** Last modified Date:      2018-08-08
** Last Version:            v1.0
** Description:             �Լ���ʧ���ʲ���
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2018-08-08
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:          
** Version:                 
** Description:             
**
*********************************************************************************************************/
/*********************************************************************************************************
** �Ƿ����ô��ڵ��Թ���
*********************************************************************************************************/
#define DEBUG 0
#if DEBUG
#include "usart.h"
#define PRINTF(...)   printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*********************************************************************************************************
	ͷ�ļ�
*********************************************************************************************************/
#include "moto_debug.h"
#include "wwdg.h"
#include "MSD_test.h"
void pwm_add(struct motor_para *motor_para)
{
	motor_para->pwm_send++;
}


void upload_step_info(uint8_t mission,uint16_t step,uint16_t setpcount)
{
	iwdg_t *wdg = &_iwdg;
	//wdg->wdg_motor_mission_set(wdg,&motor);
	motor.running_state = M_RESET_START;
	motor.current_mission = MOTOR_RESET;
	
	motor.recover_times ++;
	if(motor.recover_times>1)
	{
	wdg->wdg_motor_mission_recovery(wdg,&motor);
	motor.running_state = MOTOR_TIME_OUT;
	motor.recover_times=0;
	}
	wdg->wdg_flag_set(wdg);
	uint8_t buf[7];
	int16_t miss_step = setpcount-step;
	if(miss_step <0 )
		miss_step=-miss_step;
	buf[0] = miss_step>>8;
	buf[1] = miss_step;
	buf[2] = setpcount >> 8;
	buf[3] = setpcount;
	buf[4] = step >> 8;
	buf[5] = step;
	action_value_send_none_80(buf,6,0x88);

}

