/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               main.c
** Last modified Date:      2018-05-18
** Last Version:            v1.1
** Description:             �Լ���������
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2017-04-09
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             ��УԴ
** Modified date:           2018-05-18
** Version:                 v1.1
** Description:             ���Ӵ��ڵ��Թ��ܣ��������������ж���
**													ͨ����USART_DEBUG_ENABLE �򿪴��ڵ��Թ��� 
**													����ʹ��printf ���� ����������������һ����ʱ��ӡ����
**
*********************************************************************************************************/


/*********************************************************************************************************
	ͷ�ļ�
*********************************************************************************************************/
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "config.h"
#include "base_driver.h"
#include "delay.h"

#include "nand_driver.h"
#define NAND_FLASH_TEST 0
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_usart_dma.h"
#include "stm32fxxx_it.h"
#include "string.h"
#include <stdio.h>
#include "temp_control.h"	
#include "b3470.h"
#include "one_dimension_code.h"
#include "can.h"
#include "MicroStepDriver.h" 
#include "MSD_test.h" 
#include "coder.h"
#include "main.h"
#include "usart_screen.h"
#include "stmflash.h"
#include "notification.h"
#include "BUZZER.h"
#include "cooler.h"
#include "adc.h"
#include "mission.h"
#include "tm_stm32f4_watchdog.h"
/*********************************************************************************************************
** �Ƿ����ô��ڵ��Թ���
*********************************************************************************************************/
#define USART_DEBUG_ENABLE 0  //��ʼ�����ڹ���

#if USART_DEBUG_ENABLE
#define DEBUG 1
uint32_t uart_debug_time;
#else
#define DEBUG 0
#endif

#if DEBUG
#include "usart.h"
#define PRINTF(...)   printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*********************************************************************************************************
 ȫ�ֱ���
*********************************************************************************************************/
char 		USB_SendReady=0;
extern  __IO uint8_t USB_StatusDataSended;
extern  uint32_t USB_ReceivedCount;
extern  uint8_t USB_Tx_Buffer[];
extern  uint8_t USB_Rx_Buffer[];
extern __IO uint8_t DeviceConfigured;

/*********************************************************************************************************
 USB ���ٹ���ģʽ
*********************************************************************************************************/
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;


#if USART_DEBUG_ENABLE
/*********************************************************************************************************
** Function name:       b3470_init
** Descriptions:        ����300msѭ����ӡ��������
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-18
*********************************************************************************************************/
void usart_debug_mission(void)
{
	if(uart_debug_time < time)
	{
		PRINTF("temp = %d \r\n",b3470_get_temperature_offset(B3470_C3));//�ַ���д�뻺��
		uart_debug_time = time + 300;
	}

}
#endif



int main(void) {
	/* Initialize system */
	u8 	canrxbuf[20];			//USB����װ֡
	u16 comandbuf[3];			//CAN2USB buffer
	u8  send_size_buf[2];	
	u8 	sendcount;
	//USB��ʼ��
	USBD_Init(&USB_OTG_dev,USB_OTG_HS_CORE_ID,&USR_desc,&USBD_CDC_cb,&USR_cb);
	//can��ʼ��
	CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,24,0);//CAN��ʼ������ģʽ,������125Kbps
#if USART_DEBUG_ENABLE
	//������ͨ��ʼ��
	USART_Configuration(9600);
#else
	//DMA���ڳ�ʼ��
	TM_USART_Init(USART1, TM_USART_PinsPack_2, 9600);
	TM_USART_DMA_Init(USART1);
#endif
	
	//�δ�ʱ�ӳ�ʼ��
	SysTick_Config(SystemCoreClock/1000);
	
	//һά��ɨ���ʼ��
	one_dimension_code_init();
	//��λ�������жϳ�ʼ��
	EXTIX_Init();
	//��������ʼ��
	configureEncoder();
	//FLASH ������ȡ
	flash_init();
	//LED ��ʼ��
	LED_init();
	//LED1 ��ʼ��
	LED1_init();	
	//�����ʼ��
	cooler_init();
	
	//�¶ȴ�������ʼ��
	b3470_init();   
	//��ʼ���������λ���ݵ�FLASH
	//position_init_to_flash();
	//���������ʼ��
	MSD_Init();
	//��Դ���Ƴ�ʼ��
	power_init();
	//��������ʼ��
	BUZZER_Init();
	//���Ź���ʼ��
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
			PRINTF("SYSTEM RESET\r\n");
	else
			PRINTF("KSM reagent-disk system start!\r\n");
	TM_WATCHDOG_Init(TM_WATCHDOG_Timeout_2s);
	
	
	while (1) {          //����ѭ��
		TM_WATCHDOG_Reset(); //ι��
#if USART_DEBUG_ENABLE
		usart_debug_mission();
#else
			//�¶���ʾ����
			temp_display_mission();
#endif

		//����������
			BUZZER_mission_polling();
		//�����λ����
			motor_move_polling();

		//�����λ����
			motor_reset();
		//�����ҡ����
		  motor_shocking();
		//��Ļʱ��ͬ������
			su_mission_polling();
		//FLASH����ƫ������
			flash_mission_polling();
		//ʵ���¶Ȳ�ѯ����
			lm75a_mission_polling();
		//LED��̬��������
			led_mission_polling();
			//ADC�¶ȶ�ȡ����
			lm75a_temp_read_polling();
			//��ά��ɨ������
			one_dimension_code_mission_polling();
			//���ά������
			motor_maintain_polling();
			
//ͨ�Ų���	
		switch(USB2CAN_STATE){ 
		
		case USB_IDLE:
				break;
			
		case USB_FRAME_GET:   //��������
				//����ָ�����Դ�С���ش���ȡ��֡��
				if(canRxMsgBufferIndex>=canRxIndex){
					comandbuf[0]=canRxMsgBufferIndex-canRxIndex;
					comandbuf[1]=canRxMsgBufferIndex;
				}else{
					comandbuf[0]=CAN_BUFFER_SIZE+canRxMsgBufferIndex-canRxIndex;
					comandbuf[1]=canRxMsgBufferIndex;
				}
				//USB�ϴ�ָʾ��
			if(comandbuf[0]>0){
				LED_ON(LED_UPLOAD);
				lmt.usb_tx_time=time+MIN_SLOT;
			}
				//ÿ�β�ѯ����ϴ�220֡
				if(comandbuf[0]<220){
				send_size_buf[0]=comandbuf[0];
				}else{
				send_size_buf[0]=220;
				}
				//���ͻ���֡��
				DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,send_size_buf,1);
				
				USB2CAN_STATE=USB_SEND_FRAME;
				sendcount=0;
				led_to_notification(LED_USB_POLLING);
				//���û�д����͵�֡��ֱ�ӷ���
				if(send_size_buf[0]==0)
					USB2CAN_STATE=USB_IDLE;
				
			  break;
				
		case USB_SEND_FRAME:
					//���ͳɹ��Ž�����һ�η���
					if(USB_StatusDataSended==0)
						break;
					USB_StatusDataSended=0;
					
					//����֡��ֹͣ����
					if(sendcount>send_size_buf[0]){
						USB2CAN_STATE=USB_IDLE;
						break;
					}
					//����֡��
					for(;sendcount<send_size_buf[0];sendcount++){
					//��֡װ�뷢�ͻ�����
					parpareUSBframe(canrxbuf);
					DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,canrxbuf,15);
					poniter_plus_one(&canRxIndex);
					}
					break;
			
		}
			//����д����͵�֡
			if(USBRxCanBufferIndex!=USBRxIndex){
				//LED����ʾ
				led_to_notification(LED_CAN_TX);
					if(time>usb_waittime){
						CAN_Transmit(CAN1, &USBRxMessage[USBRxIndex]); 
						USBRxIndex++;
						//1ms֡����
						usb_waittime=time+1;
					}
					
			}
					
		}
	
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
