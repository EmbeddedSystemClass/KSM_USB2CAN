#include "can.h"
#include "delay.h"
#include "usart.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include <string.h>
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_usart_dma.h"
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
extern char USART_Buffer[100];

uint8_t USB2CAN_STATE;
CanRxMsg  CanRxMessage[CAN_BUFFER_SIZE];
CanTxMsg	USBRxMessage[255];

uint8_t USBRxCanBufferIndex=0;
uint8_t USBRxIndex=0;

uint16_t canRxMsgBufferIndex=0;
uint16_t canRxIndex=0;

can_mission_t can_mission;
uint8_t mission_state[255];

static void can_rec_index_add(){
	//�������ָ�������󻺴泤�ȣ�����0
		
	if(canRxMsgBufferIndex>=CAN_BUFFER_SIZE-1){
		canRxMsgBufferIndex=0;
	}else{
		canRxMsgBufferIndex++;
	}
	
}



void poniter_plus_one(u16 * index){
	if(*index>=CAN_BUFFER_SIZE){
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

//can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)	
//len:���ݳ���(���Ϊ8)				     
//msg:����ָ��,���Ϊ8���ֽ�.
//����ֵ:0,�ɹ�;
//		 ����,ʧ��;
u8 CAN1_Send_Msg(void)
{	
  u8 mbox;
  u16 i=0;
	u16 j=0;
  CanTxMsg TxMessage;
	TxMessage.StdId=(USB_Rx_Buffer[0]<<24)|(USB_Rx_Buffer[1]<<16)|(USB_Rx_Buffer[2]<<8)|(USB_Rx_Buffer[3]<<0);
  TxMessage.ExtId=(USB_Rx_Buffer[0]<<24)|(USB_Rx_Buffer[1]<<16)|(USB_Rx_Buffer[2]<<8)|(USB_Rx_Buffer[3]<<0);	 // ������չ��ʾ����29λ��
  TxMessage.IDE=USB_Rx_Buffer[4];		  // ʹ����չ��ʶ��
  TxMessage.RTR=USB_Rx_Buffer[5];		  // ��Ϣ����Ϊ����֡��һ֡8λ
  TxMessage.DLC=USB_Rx_Buffer[6];							 // ������֡��Ϣ
	j=7;
  for(i=0;i<USB_Rx_Buffer[6];i++)
  TxMessage.Data[i]=USB_Rx_Buffer[j+i];				 // ��һ֡��Ϣ          
  mbox=CAN_Transmit(CAN1, &TxMessage);   
  i=0;
  while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	//�ȴ����ͽ���
  if(i>=0XFFF)return 1;
  return 0;		

}
//can�ڽ������ݲ�ѯ
//buf:���ݻ�����;	 
//����ֵ:0,�����ݱ��յ�;
//		 ����,���յ����ݳ���;
u8 CAN1_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	u8  j=0;
//	CanRxMsg RxMessage;
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		//û�н��յ�����,ֱ���˳� 
    CAN_Receive(CAN1, CAN_FIFO0, &CanRxMessage[canRxMsgBufferIndex]);//��ȡ����	
	printf("can dealing mes %d \n",canRxMsgBufferIndex);

	  buf[0]=CanRxMessage[canRxMsgBufferIndex].StdId>>24;
		buf[1]=CanRxMessage[canRxMsgBufferIndex].StdId>>16;
		buf[2]=CanRxMessage[canRxMsgBufferIndex].StdId>>8;
		buf[3]=CanRxMessage[canRxMsgBufferIndex].StdId>>0; 
	  buf[4]=CanRxMessage[canRxMsgBufferIndex].IDE;
		buf[5]=CanRxMessage[canRxMsgBufferIndex].RTR;
		buf[6]=CanRxMessage[canRxMsgBufferIndex].DLC;
	  j=7;
    for(i=0;i<CanRxMessage[canRxMsgBufferIndex].DLC;i++)
    buf[j+i]=CanRxMessage[canRxMsgBufferIndex].Data[i];      
	
		canRxMsgBufferIndex++;
	
	return (CanRxMessage[canRxMsgBufferIndex-1].DLC+7);	
	
}

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

void can_mesage_send_polling(){
	char i=0;
	char can_frame_num=0;
	char can_last_frame_byte;
	if(can_mission.mission_state==CAN_MISSION_FINISHED && can_mission.current_mission!=CAN_MISSION_IDLE){
			
			//Ԥ��can֡
			can_frame_num=(can_mission.data_length/7)+1;
			//���һ֡�ĳ���
			can_last_frame_byte=can_mission.data_length%7;
			//�ر��ж�
			CAN_ITConfig(CAN1,CAN_IT_FMP0,DISABLE);
			//��¼��ʼ��֡
			can_mission.can_buffer_index=canRxMsgBufferIndex;
			//����֡�ж�
			if(can_last_frame_byte==0)
			{
			canRxMsgBufferIndex+=can_frame_num-1;
			}else{
				canRxMsgBufferIndex+=can_frame_num;
			}
			//�����ж�
			 CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);
			//�����֡
			for(i=0;i<can_frame_num-1;i++){
				memcpy(CanRxMessage[can_mission.can_buffer_index].Data+1,can_mission.buf+i*7,8);
				CanRxMessage[can_mission.can_buffer_index].StdId=CAN_MASTER_ID;
				CanRxMessage[can_mission.can_buffer_index].DLC=8;
				CanRxMessage[can_mission.can_buffer_index].Data[0]=can_mission.current_mission;
				can_mission.can_buffer_index++;
			}
			//	canRxMsgBufferIndex++;
				if(can_last_frame_byte!=0){
				CanRxMessage[can_mission.can_buffer_index].StdId=CAN_MASTER_ID;
				CanRxMessage[can_mission.can_buffer_index].DLC=can_last_frame_byte+1;
				CanRxMessage[can_mission.can_buffer_index].Data[0]=can_mission.current_mission;
				memcpy(CanRxMessage[can_mission.can_buffer_index].Data+1,can_mission.buf+i*7,(can_last_frame_byte+1));
				}
				sprintf(USART_Buffer,"can_frame_num %d can_last_frame_byte %d can_mission.can_buffer_index %d canRxMsgBufferIndex%d\n",can_frame_num,can_last_frame_byte,can_mission.can_buffer_index,canRxMsgBufferIndex);
				TM_USART_DMA_Send(USART1, (uint8_t *)USART_Buffer, strlen(USART_Buffer));
			
				can_mission.current_mission=CAN_MISSION_IDLE;
				can_mission.mission_state=CAN_MISSION_IDLE;
		}
	
	
	
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

u8 mission_idle_wait(u8 mission){
	if(can_mission.current_mission==CAN_MISSION_IDLE)
	{
		can_mission.current_mission=mission;
		return 1;
	}
			return 0;
}



 








