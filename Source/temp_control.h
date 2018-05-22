/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               temp_control.h
** Last modified Date:      2018-05-22
** Last Version:            v1.1
** Description:             
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2017-05-16
** Version:                 v1.0
** Descriptions:            �Լ����¶ȿ��Ƴ���
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             ��УԴ
** Modified date:           2018-05-22
** Version:                 v1.1
** Description:             
**
*********************************************************************************************************/
#ifndef __TEMP_CONTROL_H_
#define __TEMP_CONTROL_H_	     
/*********************************************************************************************************
	ͷ�ļ�
*********************************************************************************************************/
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_usart_dma.h"
#include "usart_screen.h"
#include "can.h"
#include "b3470.h"
#include "adc.h"
#include "stmflash.h"
/*********************************************************************************************************
	�¶ȶ�ȡ��غ궨��
*********************************************************************************************************/
#define LM75_IDLE 				0	
#define LM75_PENGDING 		1
#define LM75_SUCCESS			2
#define LM75_TX						3
#define LM75_FALIED				4

#define LM75_TIMEOUT			2000
#define LM75PRED_TIME 		6000//��ȡ�¶�ʱ������
#define LM75WAITIME 			50
#define LM75_MAX_RETRY		5
/*********************************************************************************************************
�¶���ʾ��غ궨��
*********************************************************************************************************/
#define LM75_ACK 								0x01

#define SCREEN_DIS_IDLE					0x00
#define SCREEN_DIS_PENDING 			0x01
#define SCREEN_DIS_QUERY    		0x02
#define SCREEN_DIS_POLLOING			0x03
#define	SCREEN_DIS_FAILED				0x04
#define SCREEN_DIS_SUCCESS			0x05
#define SCREEN_DIS_START				0x06
#define SCREEN_DIS_START_CHECK	0x07

/*********************************************************************************************************
�¶����������غ궨��
*********************************************************************************************************/
#define ZL_WD_H 75            //���俪���¶�
#define ZL_WD_L 45            //����ر��¶�
#define C3_DIPALY_ONLY		0

#define LM35_READ_IDLE 		0x00
#define LM35_READ_START 	0x01
#define	LM35_READ_FINISH	0x02
#define COOL_MAX_TIME			10000 		//�����¶�
#define	COOL_STOP_TIME		5000		
#define USE_LM35 0				//�Ƿ�ʹ��LM35 �¶ȴ�����  1:LM32 0:B3470

/*********************************************************************************************************
  ȫ�ֱ������� �Լ��ⲿ����
*********************************************************************************************************/
typedef struct {
	u8 	current_mission;  										//mission type
	u8	mission_state;         								//��ƽת��ʱ��
	u8	index;                                //�����������ܳɹ�֮�󷵻�����֮��Ҳ���ǵ�Ƭ���ɹ�����һ�������idex�ͼ�1
	u8	retry_count;
	u8  Integred;
	u8  decide;
	u8  rank;
	u32 waitime;
	u32	timeout;
	u32 periodtime;
}LM75_usart_t;

typedef struct {
	u8	mission_state;         								
	u32 waitime;
	u16 temp;
	u16 temp_real;
	u16 temp_buffer[10];
	u8 cooler_function;
	u8 c3_control_cooler;
	u32	pwm_time;
	u8 times;
	u32 temp_all;
	u8 temp_high;
	u8 temp_alarm_state;
	u32 close_inter_fan_time;
	u8 close_inter_fan_enable;
	u32 cooler_timeout;					//��Сʱ�رշ��Ⱥ��
}lm35_temp_t;
extern u8 gobal_temp_flag;
extern LM75_usart_t LM75t;
extern volatile lm35_temp_t lm35_t;
/*********************************************************************************************************
  �ⲿ�����Լ���������
*********************************************************************************************************/
u8 lm75a_init(void);
float lm75a_temp_read(void);
void add_end_code(char *buff);
void screen_time_polling(void);
void lm75a_usart_polling(void);
u8 LM75_parpare_buffer(void);
void temp_display_mission(void);
extern u8	lm75_status;
void lm75a_mission_polling(void);
void lm75a_temp_read_polling(void);
void bubble_sort_better(__IO u16 a[],u16 n);
u16 temp_faded(u16 temp , u16 max_temp);
void reser_screen(void);
#endif
