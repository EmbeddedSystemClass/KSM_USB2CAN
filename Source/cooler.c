/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               cooler.c
** Last modified Date:      2018-05-23
** Last Version:            v1.2
** Description:             
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
#include "cooler.h"
#include "temp_control.h"	
#include "notification.h"
/*********************************************************************************************************
	ȫ�ֱ���
*********************************************************************************************************/
u8 power_off_state;
u8 cooler_received_command;		//�Ƿ��յ������俪ʼ����
extern u32 time;

/*********************************************************************************************************
** Function name:       cooler_init
** Descriptions:        ����Ƭ��ʼ�����˿�����
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-18
*********************************************************************************************************/
void	cooler_init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	
		COOLER_AHBxClock_FUN(COOLER_GPIO_CLK,ENABLE);//����ʱ��
	
		GPIO_InitStructure.GPIO_Pin =  COOLER_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(COOLER_PORT, &GPIO_InitStructure);
		
		cooler_off(); //��ʼ���ر�����
}

/*********************************************************************************************************
** Function name:       cooler_on
** Descriptions:        ���俪�� �͵�ƽ ���ڲ����� ����ָʾ��
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/
void 	cooler_on(void)
{
	GPIO_ResetBits(COOLER_PORT,COOLER_PIN);
	open_inter_fan();
	LED_ON(COOLER_LED);
	lm35_t.c3_control_cooler=1;
	lm35_t.close_inter_fan_enable=0;
}


/*********************************************************************************************************
** Function name:       cooler_off
** Descriptions:       	����ر� �ߵ�ƽ �ر�ָʾ��
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/                                                                        // 11������Ƭ��12��ˮ��ã�13�Ƿ���1-ˮ����ȣ�14�Ƿ���3-�ڲ�����
void 	cooler_off(void){   //�ر�������庯��
	  GPIO_SetBits(COOLER_PORT, GPIO_Pin_11);    //�ر�����Ƭ
		LED_OFF(COOLER_LED);
		lm35_t.close_inter_fan_enable=1;
		lm35_t.close_inter_fan_time=time+10000;
}

/*********************************************************************************************************
** Function name:       close_inter_fan
** Descriptions:       	�ر��ڲ�����
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/   
void close_inter_fan(void){
	GPIO_SetBits(COOLER_PORT, GPIO_Pin_14);    //�ر��ڲ����� 
	PRINTF("�ر��ڲ����� \r\n");
}

/*********************************************************************************************************
** Function name:       open_inter_fan
** Descriptions:       	�����ڲ�����
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/  
void open_inter_fan(void){
	GPIO_ResetBits(COOLER_PORT, GPIO_Pin_14);    //�ر��ڲ����� 
	PRINTF("�����ڲ����� \r\n");
}


/*********************************************************************************************************
** Function name:       power_init
** Descriptions:       	��Դ����-�̵�����ʼ��
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/  
void power_init(void){
		GPIO_InitTypeDef GPIO_InitStructure;
		POWER_AHBxClock_FUN(POWER_GPIO_CLK,ENABLE);  //ʱ��ʹ��
	//�˿�����
		GPIO_InitStructure.GPIO_Pin =  POWER_PIN;     //PD4 �̵�����������
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(POWER_PORT, &GPIO_InitStructure);
		power_on();
}

/*********************************************************************************************************
** Function name:       power_on
** Descriptions:       	��Դ����-���̵���
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/  
void 	power_on(void){   //���̵���
	GPIO_ResetBits(POWER_PORT,POWER_PIN);
	LED_ON(LED_POWER);
}

/*********************************************************************************************************
** Function name:       power_off
** Descriptions:       	��Դ����-�رռ̵���
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/ 
void 	power_off(void){
	GPIO_SetBits(POWER_PORT,POWER_PIN);   //�رռ̵���  PD4
	LED_OFF(LED_POWER);  
}

/*********************************************************************************************************
** Function name:       cooler_pwm_mission
** Descriptions:        ÿ����Сʱ�ر�����Ƭ1����
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-23
*********************************************************************************************************/ 
void 	cooler_pwm_mission(void)
{ 
	switch(lm35_t.cooler_pwm_function)
	{
		case COOLER_OFF:
		break;
		
		case COOLER_UP:
			if(time>lm35_t.pwm_time)
			{
				PRINTF("PWM �¶ȿ������� \r\n");
				cooler_on();
				lm35_t.cooler_pwm_function=COOLER_DOWN;
				lm35_t.cooler_function=1;
				lm35_t.pwm_time=time+flash_get_para(FLASH_LM35_ON_TIME)*1000;
				break;
			}
		break;
			
		case	COOLER_DOWN:
				if(time>lm35_t.pwm_time)
				{
				PRINTF("PWM �¶ȹر����� \r\n");
			  cooler_off();
				lm35_t.cooler_function=0;
				lm35_t.cooler_pwm_function=COOLER_UP;
				lm35_t.pwm_time=time+flash_get_para(FLASH_LM35_OFF_TIME)*1000;
				break;
				}
		break;
	}
}
