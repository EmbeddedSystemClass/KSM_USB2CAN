/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               can.c
** Last modified Date:      2018-05-23
** Last Version:            v1.1
** Description:             can������ع���
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2017-05-16
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             ��УԴ
** Modified date:           2018-05-23
** Version:                 v1.1
** Description:             ɾ�����ú���
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
#include "stmflash.h"
#include "can.h"
#include "MSD_test.h"  
#include "can.h"
#include "usart.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include <string.h>
#include "MSD_test.h"  
#include "notification.h"

//usbd_cdc_core 530 ֡��ʽ
//////////////////////////////////////////////////////////////////////////////////	 

//CAN��ʼ��
//tsjw:����ͬ����Ծʱ�䵥Ԫ.��Χ:CAN_SJW_1tq~ CAN_SJW_4tq
//tbs2:ʱ���2��ʱ�䵥Ԫ.   ��Χ:CAN_BS2_1tq~CAN_BS2_8tq;
//tbs1:ʱ���1��ʱ�䵥Ԫ.   ��Χ:CAN_BS1_1tq ~CAN_BS1_16tq
//brp :�����ʷ�Ƶ��.��Χ:1~1024; tq=(brp)*tpclk1
//������=Fpclk1/((tbs1+1+tbs2+1+1)*brp);
//mode:CAN_Mode_Normal,��ͨģʽ;CAN_Mode_LoopBack,�ػ�ģʽ;
//Fpclk1��ʱ���ڳ�ʼ����ʱ������Ϊ42M,�������CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_Mode_LoopBack);
//������Ϊ:42M/((6+7+1)*6)=500Kbps
//����ֵ:0,��ʼ��OK;
//    ����,��ʼ��ʧ��; 
extern  uint8_t USB_Tx_Buffer[];
extern  uint8_t USB_Rx_Buffer[];

/*********************************************************************************************************
	RAM2ȫ�ֱ���   ����������
*********************************************************************************************************/
CanRxMsg  CanRxMessage[CAN_BUFFER_SIZE] 		__attribute__((at(0x10000218)));
CanTxMsg	USBRxMessage[255] 								__attribute__((at(0x10002928)));
uint8_t 	USBRxCanBufferIndex								__attribute__((at(0x10000008)));
uint8_t 	USBRxIndex 												__attribute__((at(0x1000000C)));
uint16_t 	canRxMsgBufferIndex 							__attribute__((at(0x10000010)));
uint16_t 	canRxIndex 												__attribute__((at(0x10000014)));
uint8_t 	temp_control 											__attribute__((at(0x10000000)));
uint8_t 	power_satus 											__attribute__((at(0x10000004)));
uint8_t 	is_soft_start 										__attribute__((at(0x10003f08)));
/*********************************************************************************************************
	ȫ�ֱ���    
*********************************************************************************************************/
uint8_t mission_state[255];
uint8_t USB2CAN_STATE;
static void can_rec_index_add(){
	//�������ָ�������󻺴泤�ȣ�����0
	if(canRxMsgBufferIndex>=CAN_BUFFER_SIZE-1)
	{
		canRxMsgBufferIndex=0;
	}
	else
	{
		canRxMsgBufferIndex++;
	}
}



void poniter_plus_one(u16 * index){
	if(*index>=CAN_BUFFER_SIZE-1){
		*index=0;
	}else{
		(*index)++;
	}

}

void poniter_add(u16 *index,u8 number){
	while(number--)
		poniter_plus_one(index);
	
}
u8 CAN1_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
{

  	GPIO_InitTypeDef GPIO_InitStructure; 
	  CAN_InitTypeDef        CAN_InitStructure;
  	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
#if CAN1_RX0_INT_ENABLE 
   	NVIC_InitTypeDef  NVIC_InitStructure;
#endif
    //ʹ�����ʱ��
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��PORTBʱ��	                   											 

  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��	
	
    //��ʼ��GPIO
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8| GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��PA11,PA12
	
	  //���Ÿ���ӳ������
	  GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_CAN1); //GPIOA11����ΪCAN1
	  GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_CAN1); //GPIOA12����ΪCAN1
	  
  	//CAN��Ԫ����
   	CAN_InitStructure.CAN_TTCM=DISABLE;	//��ʱ�䴥��ͨ��ģʽ   
  	CAN_InitStructure.CAN_ABOM=DISABLE;	//����Զ����߹���	  
  	CAN_InitStructure.CAN_AWUM=DISABLE;//˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)
  	CAN_InitStructure.CAN_NART=DISABLE;	//��ֹ�����Զ����� 
  	CAN_InitStructure.CAN_RFLM=DISABLE;	//���Ĳ�����,�µĸ��Ǿɵ�  
  	CAN_InitStructure.CAN_TXFP=DISABLE;	//���ȼ��ɱ��ı�ʶ������ 
  	CAN_InitStructure.CAN_Mode= mode;	 //ģʽ���� 
  	CAN_InitStructure.CAN_SJW=tsjw;	//����ͬ����Ծ���(Tsjw)Ϊtsjw+1��ʱ�䵥λ CAN_SJW_1tq~CAN_SJW_4tq
  	CAN_InitStructure.CAN_BS1=tbs1; //Tbs1��ΧCAN_BS1_1tq ~CAN_BS1_16tq
  	CAN_InitStructure.CAN_BS2=tbs2;//Tbs2��ΧCAN_BS2_1tq ~	CAN_BS2_8tq
  	CAN_InitStructure.CAN_Prescaler=brp;  //��Ƶϵ��(Fdiv)Ϊbrp+1	
  	CAN_Init(CAN1, &CAN_InitStructure);   // ��ʼ��CAN1 
    
		//���ù�����
 	  CAN_FilterInitStructure.CAN_FilterNumber=0;	  //������0
  	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; 
  	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32λ 
  	CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;////32λID
  	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
  	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;//32λMASK
  	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
   	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//������0������FIFO0
  	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //���������0
  	CAN_FilterInit(&CAN_FilterInitStructure);//�˲�����ʼ��
		
#if CAN1_RX0_INT_ENABLE
	
	  CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0��Ϣ�Һ��ж�����.		    
  
  	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // �����ȼ�Ϊ1
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // �����ȼ�Ϊ0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
#endif
	return 0;
}   
 
#if CAN1_RX0_INT_ENABLE	//ʹ��RX0�ж�
//�жϷ�����			    
void CAN1_RX0_IRQHandler(void)
{
	/*
  	CanRxMsg RxMessage;
	int i=0;
    CAN_Receive(CAN1, 0, &RxMessage);
	for(i=0;i<8;i++)
	printf("rxbuf[%d]:%d\r\n",i,RxMessage.Data[i]);
	*/
	
	CAN_Receive(CAN1, CAN_FIFO0, &CanRxMessage[canRxMsgBufferIndex]);
	//canRxMsgBufferIndex++;
	can_rec_index_add();
	led_to_notification(LED_CAN_RX);
	//if(canRxMsgBufferIndex>=canSendedIndex)canRxMsgBufferIndex++;

}
#endif


void parpareUSBframe(u8 *buf){
		u8 j,i;
		buf[0]=CanRxMessage[canRxIndex].StdId>>24;
		buf[1]=CanRxMessage[canRxIndex].StdId>>16;
		buf[2]=CanRxMessage[canRxIndex].StdId>>8;
		buf[3]=CanRxMessage[canRxIndex].StdId>>0; 
	  buf[4]=CanRxMessage[canRxIndex].IDE;
		buf[5]=CanRxMessage[canRxIndex].RTR;
		buf[6]=CanRxMessage[canRxIndex].DLC;
	  j=7;
    for(i=0;i<CanRxMessage[canRxIndex].DLC;i++)
    buf[j+i]=CanRxMessage[canRxIndex].Data[i];      
}



void can_mutil_frame_send(u8 *buf,u8 length,u8 mission){
	char i=0;
	char can_frame_num=0;
	char can_last_frame_byte=0;
	u16	can_index_start=0;
			
			//Ԥ��can֡
			can_frame_num=(length/CAN_DATA_LENGTH)+1;
			//���һ֡�ĳ���
			can_last_frame_byte=length%CAN_DATA_LENGTH;
			//��¼��ʼ��֡
	
			can_index_start=canRxMsgBufferIndex;
			//����֡�ж�
			CAN_ITConfig(CAN1,CAN_IT_FMP0,DISABLE);
			if(can_last_frame_byte==0)
			{
				poniter_add(&canRxMsgBufferIndex,can_frame_num-1);
				//canRxMsgBufferIndex+=can_frame_num-1;
			}else{
				poniter_add(&canRxMsgBufferIndex,can_frame_num);
				//canRxMsgBufferIndex+=can_frame_num;
			}
			CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);
			//�����֡
			for(i=0;i<can_frame_num-1;i++){
				memcpy(CanRxMessage[can_index_start].Data+CAN_DATE_START_BIT,buf+i*CAN_DATA_LENGTH,CAN_DATA_LENGTH);
				CanRxMessage[can_index_start].StdId=CAN_MASTER_ID;
				CanRxMessage[can_index_start].DLC=8;
				CanRxMessage[can_index_start].Data[CAN_MISSION_BIT]=mission;
				CanRxMessage[can_index_start].Data[CAN_COMPITE_BIT]=CAN_COMPITE_CONTENT;
				CanRxMessage[can_index_start].Data[CAN_FRAME_FRAG_BIT]=i+1;
				
				poniter_plus_one(&can_index_start);
			//	can_index_start++;
			}
			
				if(can_last_frame_byte!=0){
				CanRxMessage[can_index_start].StdId=CAN_MASTER_ID;
				CanRxMessage[can_index_start].DLC=can_last_frame_byte+CAN_DATE_START_BIT;
				CanRxMessage[can_index_start].Data[CAN_MISSION_BIT]=mission;
				CanRxMessage[can_index_start].Data[CAN_COMPITE_BIT]=CAN_COMPITE_CONTENT;
				CanRxMessage[can_index_start].Data[CAN_FRAME_FRAG_BIT]=i+1;
				memcpy(CanRxMessage[can_index_start].Data+CAN_DATE_START_BIT,buf+i*CAN_DATA_LENGTH,(can_last_frame_byte+1));
				}
			//	sprintf(USART_Buffer,"can_frame_num %d can_last_frame_byte %d can_mission.can_buffer_index %d canRxMsgBufferIndex%d\n",can_frame_num,can_last_frame_byte,can_mission.can_buffer_index,canRxMsgBufferIndex);
			//	TM_USART_DMA_Send(USART1, (uint8_t *)USART_Buffer, strlen(USART_Buffer));
		
}

void  action_success_send(void){
	u8 buf[1];
	buf[0]=ACTION_SUCCESS;
	mission_state[motor.current_mission]=ACTION_SUCCESS;
	one_can_frame_send(buf,1,motor.current_mission);
}
void action_failed_send(void){
	u8 buf[3];
	buf[0]=ACTION_FAILED;
	buf[1]=motor.running_state;
	mission_state[motor.current_mission]=ACTION_FAILED;
	one_can_frame_send(buf,2,motor.current_mission);
}
void  mission_success_send(u8 mission){
	u8 buf[1];
	buf[0]=ACTION_SUCCESS;
	mission_state[mission]=ACTION_SUCCESS;
	one_can_frame_send(buf,1,mission);
}

void  mission_failed_send(u8 mission){
	u8 buf[1];
	buf[0]=ACTION_FAILED;
	mission_state[mission]=ACTION_FAILED;
	one_can_frame_send(buf,1,mission);
}


void action_value_send(u8 *buf,u8 length,u8 mission){
	u8 buffer[8];
	u8 i;
	buffer[0]=ACTION_SUCCESS;
	
	mission_state[mission]=ACTION_SUCCESS;
	
	for(i=0;i<length;i++)
	buffer[1+i]=buf[i];
	
	one_can_frame_send(buffer,length+1,mission);
}

void action_value_send_none_80(u8 *buf,u8 length,u8 mission){
	u8 buffer[8];
	u8 i;
	mission_state[mission]=ACTION_SUCCESS;
	for(i=0;i<length;i++)
	buffer[i]=buf[i];
	one_can_frame_send(buffer,length,mission);
}


void	one_can_frame_send(u8 *buf,u8 length,u8 mission){
	u16 current_frame_index;
	
	CAN_ITConfig(CAN1,CAN_IT_FMP0,DISABLE);
		current_frame_index=canRxMsgBufferIndex;
	
		poniter_plus_one(&canRxMsgBufferIndex);
	
	/*
	while(canRxMsgBufferIndex!=current_frame_index+1){
			current_frame_index=canRxMsgBufferIndex;
			poniter_plus_one(&canRxMsgBufferIndex);
	}
	*/
	
	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);
	
	CanRxMessage[current_frame_index].StdId=CAN_MASTER_ID;
	CanRxMessage[current_frame_index].Data[0]=mission;
	CanRxMessage[current_frame_index].DLC=length+1;
	memcpy(CanRxMessage[current_frame_index].Data+1,buf,length+1);
}




 








