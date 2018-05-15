#ifndef __NOTIFICATION_H
#define __NOTIFICATION_H
#include "stm32f4xx.h"


#define COOLER_LED 				GPIO_Pin_8	//����
#define LED_COOLER_PWM 		GPIO_Pin_9	//����ͨ��
#define LED_POWER 				GPIO_Pin_10	//��Դ
#define LED_UPLOAD				GPIO_Pin_11	//USB�ϴ�
#define LED_CAN_SEND 			GPIO_Pin_12	//CAN����
#define LED_CAN 					GPIO_Pin_13	//CAN��ȡ
#define LED_USB_TX 				GPIO_Pin_14	//��λ���������״̬ ����˸������
#define LED_USB 					GPIO_Pin_15	//USB����״̬
#define LED_PIN   			 GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15

#define RED_LED1				  GPIO_Pin_6  //�Լ���״ָ̬ʾ�ƣ���ɫ������ʾ�������ֲ�����PC6����Ӳ������Z1-JB1-V3.2;M2�ӿڣ�2��=��ɫVCC��3��=��ɫ��4��=��ɫ
#define GREEN_LED1			  GPIO_Pin_7  //�Լ���״ָ̬ʾ�ƣ���ɫ���������ֲ�����PC7��
#define LED1_PIN 					GPIO_Pin_6|GPIO_Pin_7



#define            LED_AHBxClock_FUN            RCC_AHB1PeriphClockCmd
#define            LED_GPIO_CLK                 RCC_AHB1Periph_GPIOE
#define            LED_PORT                     GPIOE

// �Լ��̹���״ָ̬ʾ�ƣ�����ΪLED1
#define            LED1_AHBxClock_FUN           RCC_AHB1PeriphClockCmd
#define            LED1_GPIO_CLK                RCC_AHB1Periph_GPIOC
#define            LED1_PORT                    GPIOC

#define MIN_SLOT 50

#define LED_IDLE 			0x00
#define LED_PENGDING	0x01
#define LED_UP				0x02
#define LED_DOWN			0x03

#define LED_MISSION_IDLE	0x00
#define LED_CAN_RX			0x01
#define	LED_USB_UPLOAD	0x02
#define	LED_CAN_TX			0x03
#define LED_USB_POLLING	0x04

#define RED_LED_t   		0x01 //��Ʊ�ţ�CANͨѶ�´���
#define GREEN_LED_t   	0x02 //�̵Ʊ�ţ�CANͨѶ�´���

void LED_init(void);
void LED_ON(uint16_t LED);
void LED_OFF(uint16_t LED);
void LED_TOGGLE(uint16_t LED);

//�Լ���״ָ̬ʾ�ƣ���ɫ������ת���ɲ�������ɫ�ɲ���
void LED1_init(void);
void LED1_ON(uint16_t LED1);
void LED1_OFF(uint16_t LED1);

typedef struct {
  u8 		current_mission;  			//mission type
	u8		mission_state;      	
	u8 		command_index;
  u32 	usb_tx_time;								//��ƽת��ʱ
	u32 	usb_rx_time;
	u32		can_rx_time;
	u32		usb_polling_time;
}led_mission_t;

extern led_mission_t lmt;
void led_to_notification(u8 mission);
void led_mission_polling(void);
#endif

