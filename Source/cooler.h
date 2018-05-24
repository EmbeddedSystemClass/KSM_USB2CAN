/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               cooler.h
** Last modified Date:      2018-05-23
** Last Version:            v1.2
** Description:             ��������Ƭ�����Զϵ����
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2017-05-16
** Version:                 v1.0
** Descriptions:            ����Ƭ ���ȷ��� ����
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             ��УԴ
** Modified date:           2018-05-23
** Version:                 v1.1
** Description:             ��������Ƭ�����Զϵ����
**
*********************************************************************************************************/
#ifndef __COOLER_H
#define __COOLER_H

#include "stm32f4xx.h"
/*********************************************************************************************************
	����ͨ�Ϻ궨��
*********************************************************************************************************/
#define COOLER_OFF					0x00
#define COOLER_UP						0x01
#define COOLER_DOWN					0x02

#define COOLER_UP_TIME			1800000  //������Сʱ
#define COOLER_DOWN_TIME		60000		 //�ر�1����

/*********************************************************************************************************
	Ӳ���궨��11������Ƭ��12��ˮ��ã�13�Ƿ���1ˮ����ȣ�14�Ƿ���3�ڲ�����
*********************************************************************************************************/
#define COOLER_PIN GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14   
#define COOLER_FS_PIN	 GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13

#define            	COOLER_AHBxClock_FUN            RCC_AHB1PeriphClockCmd
#define            	COOLER_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define            	COOLER_PORT                     GPIOD
#define							COOLER_PWM_PIN									GPIO_Pin_14

#define            	POWER_AHBxClock_FUN            	RCC_AHB1PeriphClockCmd
#define            	POWER_GPIO_CLK                 	RCC_AHB1Periph_GPIOD
#define 						POWER_PORT											GPIOD
#define							POWER_PIN												GPIO_Pin_4    //PD4 �̵�����������

/*********************************************************************************************************
�ⲿ��������������
*********************************************************************************************************/
extern u8 power_off_state;
extern u8 cooler_received_command;
void	cooler_init(void);
void 	cooler_on(void);
void 	cooler_off(void);
void close_inter_fan(void);
void close_cooler_fan(void);
void power_init(void);
void 	power_on(void);
void 	power_off(void);
void 	cooler_pwm_mission(void);
void open_inter_fan(void);
#endif
