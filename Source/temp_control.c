/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               temp_control.c
** Last modified Date:      2018-05-23
** Last Version:            v1.2
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
** Description:             ����B3470 �¶ȴ��������Ʒ���
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             ��УԴ
** Modified date:           2018-05-23
** Version:                 v1.2
** Description:             ���������ϴ�bug 
**													�޸��ڲ����ȹرպ����´�bug
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
#include "BUZZER.h"
#include "temp_control.h"	
/*********************************************************************************************************
	ȫ�ֱ���
*********************************************************************************************************/
extern u32 time;
u32 lm75a_time=0;
u8	lm75_status=0;
u8 gobal_temp_flag=0;  
u16 gobal_temp = 0;
extern u8 usart_state;
char LM75_tx_buffer[20];
char LM75_rx_buffer[20];

LM75_usart_t LM75t;
volatile lm35_temp_t lm35_t;

/*********************************************************************************************************
** Function name:       lm75a_temp_read_polling
** Descriptions:        �¶ȶ�ȡ���� �����Զ�ȡ
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-22
*********************************************************************************************************/
void lm75a_temp_read_polling(void){
	switch(lm35_t.mission_state){          //���� ����״̬��ִ��
		
		case LM35_READ_IDLE:
			if(time>lm35_t.waitime){
			lm35_t.mission_state=LM35_READ_START;
				lm35_t.times=0;
				lm35_t.temp_all=0;
			}
			break;
			
		case LM35_READ_START:                   //��ʼ��ȡ�¶�
			
#if USE_LM35
		lm35_t.temp_buffer[lm35_t.times]=Get_Adc_Average(ADC_Channel_2,100); 
#else
		lm35_t.temp_buffer[lm35_t.times]=b3470_get_temperature_offset(B3470_C3); 
#endif 		
			lm35_t.times++;                 //��1                                                          
			if(lm35_t.times>2)
					lm35_t.mission_state=LM35_READ_FINISH;

			break;
		
		case LM35_READ_FINISH:                   //�¶ȶ����Ĵ���
		for(u8 i=0;i<lm35_t.times;i++){
			lm35_t.temp_all+=lm35_t.temp_buffer[i];
		}
		
		lm35_t.temp_real=lm35_t.temp_all/lm35_t.times;  //�����¶���ƽ��  -3 ��Ϊ�˽������        temp_real ����ʵ�¶�
		//ֻ��LM35 �Ž����¶Ƚ���
#if USE_LM35
		lm35_t.temp=temp_faded(lm35_t.temp_real,150);   //********�����¶Ƚ���         temp  �ǵ��ý�������ʾ�¶�	
#else
		lm35_t.temp=lm35_t.temp_real;
#endif 	
			//�����¶���������Χ�ڽ���
	  
		lm35_t.mission_state=LM35_READ_IDLE;
		lm35_t.waitime=time+1000;
	
		
		if(lm35_t.temp_real<70)
		{             //�����¶�ѡ��  ������ ����״̬
			 lm35_t.temp_alarm_state=1;
		}
		
		if(lm35_t.temp_alarm_state==1)
		if(lm35_t.temp_real>150)
		{
			buzzer_mission(MODE_THREE);
			lm35_t.temp_alarm_state=0;
		}
		
		if(time>lm35_t.close_inter_fan_time && lm35_t.close_inter_fan_enable)   //�ڲ����ȹرձ�־ ����  �� ʱ�����60S   �ر��ڲ�����
		{
			close_inter_fan();
			lm35_t.close_inter_fan_enable=0;
		}
		
#if !USE_LM35
		if(lm35_t.cooler_function)
		{    //������������
#if C3_DIPALY_ONLY
			if(b3470_get_temperature_offset(B3470_C2)<flash_get_para(FLASH_C2_ZL_LOW))
			{
				PRINTF("�ر�����C2 %d %d\r\n",b3470_get_temperature_offset(B3470_C2),flash_get_para(FLASH_C3_ZL_LOW));
				cooler_off();
				lm35_t.close_inter_fan_time=time+60000;  //��ʱ60S
				lm35_t.close_inter_fan_enable =1;        //�ر��ڲ����ȱ�־����
				//����ʱ����ʱ�رշ���
				
			}
			if(b3470_get_temperature_offset(B3470_C2)>flash_get_para(FLASH_C2_ZL_HIGH))
			{
				PRINTF("��������C2 %d %d\r\n",b3470_get_temperature_offset(B3470_C2),flash_get_para(FLASH_C3_ZL_HIGH));
				cooler_on();
				lm35_t.close_inter_fan_enable=0;     //�ر��ڲ����ȱ�־Ϊ���ɹ�
			}
#else 
			if(lm35_t.temp_real<flash_get_para(FLASH_C3_ZL_LOW))
			{
				
				//����ʱ����ʱ�رշ���
				PRINTF("C3�ر��¶�\r\n");
				lm35_t.c3_control_cooler = 0;
				cooler_off();
				lm35_t.close_inter_fan_time=time+60000;  //��ʱ60S
				lm35_t.close_inter_fan_enable =1;        //�ر��ڲ����ȱ�־����
				
			}
			if(lm35_t.temp_real>flash_get_para(FLASH_C3_ZL_HIGH))
			{
				PRINTF("C3�����¶�\r\n");
				lm35_t.c3_control_cooler = 1;
			}
#endif
		}
		if(lm35_t.c3_control_cooler & lm35_t.cooler_function)
		{
			if(b3470_get_temperature_offset(B3470_C2)<flash_get_para(FLASH_C2_ZL_LOW))
			{
				PRINTF("�ر�����C2 %d %d %d\r\n",b3470_get_temperature_offset(B3470_C2),flash_get_para(FLASH_C3_ZL_LOW),flash_get_para(FLASH_C2_STOP_TIME)*1000);
				cooler_off();
				lm35_t.close_cooler_time = time +flash_get_para(FLASH_C2_STOP_TIME)*1000;
				lm35_t.close_inter_fan_time=time+60000;  //��ʱ60S
				lm35_t.close_inter_fan_enable =1;        //�ر��ڲ����ȱ�־����
				lm35_t.close_cooler_enable =1;        //�ر��ڲ����ȱ�־����
				//����ʱ����ʱ�رշ���
				
			}
			if(b3470_get_temperature_offset(B3470_C2)>flash_get_para(FLASH_C2_ZL_HIGH) || (time > lm35_t.close_cooler_time && lm35_t.close_cooler_enable) )
			{
				PRINTF("��������C2 %d %d\r\n",b3470_get_temperature_offset(B3470_C2),flash_get_para(FLASH_C3_ZL_LOW));
				cooler_on();
				lm35_t.close_inter_fan_enable=0;     //�ر��ڲ����ȱ�־Ϊ���ɹ�
			}
		}
#else 
		if(lm35_t.cooler_function)
		{
		
		if(lm35_t.temp_real<ZL_WD_L)
			{
				
				//����ʱ����ʱ�رշ���
				PRINTF("LM35�ر��¶� %d\r\n",lm35_t.temp_real);
				cooler_off();
				lm35_t.close_inter_fan_time=time+60000;  //��ʱ60S
				lm35_t.close_inter_fan_enable =1;        //�ر��ڲ����ȱ�־����
				
			}
			if(lm35_t.temp_real>ZL_WD_H)
			{
				cooler_on();
				lm35_t.close_inter_fan_enable=0;     //�ر��ڲ����ȱ�־Ϊ���ɹ�
				PRINTF("LM35�����¶�%d\r\n",lm35_t.temp_real);
			}
		}
#endif
		
		
		
		break;
		default:
			lm35_t.mission_state = LM35_READ_IDLE;
			break;
	}
	
}


u16 temp_faded(u16 temp , u16 max_temp){   //�¶Ƚ�����򣨲�ѯ�¶�ʱ���յ���ʵ�¶ȣ�����¶ȣ�
		//�������15�� ��־Ϊ0����������¶�
		if(temp>=max_temp)
		{           
			gobal_temp =temp;
			gobal_temp_flag=0;
			return gobal_temp;
		}
  
		//С��15��ʱ��
		if(!gobal_temp_flag)
		{          //��ȡ�¶�ֵ  gobal_temp_flagĬ��Ϊ0
			gobal_temp_flag=1;
			gobal_temp = temp;
		}
		
		if(gobal_temp_flag)
		{             //������¶���ȣ����䣬������0.1�����£�����б仯��0.2Ϊ��λ�仯
			if(gobal_temp == temp)
				return gobal_temp;
			
			if((gobal_temp - temp == 1) || (temp - gobal_temp == 1))
			{
				gobal_temp = temp;
				return gobal_temp;
			}
				
			if(gobal_temp < temp)
			{
				gobal_temp += 2;
				return gobal_temp;
			}	
				
			if(gobal_temp > temp)
			{
				gobal_temp -= 2;
				return gobal_temp;
			}			
		}
   return temp;
}


// *************************** �¶���ʾ������***************************//
void temp_display_mission(void){
	float temp;
	u8 length=10;
	switch(LM75t.mission_state){
		
		case SCREEN_DIS_IDLE:
			if(usart_state==0)
				 break;
					
			if(time>=LM75t.periodtime && sut.mission_state == SCREEN_IDLE)    //��ʾ�������ж�
					LM75t.mission_state=SCREEN_DIS_PENDING;	
			break;
					
			
			
			
			
		case SCREEN_DIS_START:
			
		if(sut.mission_state!=SCREEN_IDLE)
			break;
		
		if(LM75t.retry_count>1){
					LM75t.index=2;
					LM75t.decide=lm35_t.temp%10;
					TM_USART_DMA_Send(USART1, (uint8_t *)LM75_tx_buffer,LM75_parpare_buffer());
				}
		else{
					 length=10;
					 sprintf(LM75_tx_buffer,"rank.val=1");//�ַ���д�뻺��
					 LM75_tx_buffer[length++]=0xFF;
					 LM75_tx_buffer[length++]=0xFF;
					 LM75_tx_buffer[length++]=0xFF;
					TM_USART_DMA_Send(USART1, (uint8_t *)LM75_tx_buffer,length);//���ڷ��ͷ�������
				}

				 
					LM75t.mission_state=SCREEN_DIS_START_CHECK;  //����Ƿ�ʼ����
					LM75t.timeout=time+200;
				//LM75t.mission_state=LM75T_IDLE;
		break;
		
		
				
				
				
		case  SCREEN_DIS_START_CHECK:     //���״̬
					if(sut.mission_state!=SCREEN_IDLE)
							break;
				
					//������ǿ��е� ���ٴγ��� ���0.2S
				if(time>LM75t.timeout){
					LM75t.retry_count++;
					LM75t.timeout=time+200;
					
					if(LM75t.retry_count>3){
						mission_failed_send(TEMP_DISPLAY);  //���Դ�������3�Σ���ʧ��
						LM75t.mission_state=SCREEN_DIS_IDLE;
						LM75t.retry_count=0;
					}
					
							LM75t.mission_state=SCREEN_DIS_START;
					break;
				}
				
				if(TM_USART_Gets(USART1,LM75_rx_buffer,5)){//����յ����ص�ȷ��֡������ʾ�ɹ�
					if(LM75_rx_buffer[0]==LM75_ACK){
						LM75t.mission_state=SCREEN_DIS_IDLE;
						usart_state=1;              //����������
						mission_success_send(TEMP_DISPLAY);
				}
			}
		
		break;
		
			
			
		case SCREEN_DIS_PENDING:
			    temp=lm35_t.temp/10;
					if(temp>12){
			    LM75t.rank=3;  //��ɫ
		      }else if((temp>8)&&(temp<=12)){
			    LM75t.rank=2;  //��ɫ
		      }else if(temp<=8){//(temp>=5)&&(temp<=8)
			    LM75t.rank=1;  //��ɫ������
		      }
					
					LM75t.rank=1;                     //???????????????????????????????????????????  Ŀǰ����Ļ�Լ��ж�
					LM75t.Integred=lm35_t.temp/10;       //�¶�����λת��
				  LM75t.decide=lm35_t.temp%10;         //�¶�С��λת��
		      LM75t.mission_state=SCREEN_DIS_QUERY;
					LM75t.waitime=time+LM75WAITIME;
					LM75t.index=1;
			break;
					
					
		case SCREEN_DIS_QUERY:  //��ʱ�����¶�
			if(sut.mission_state != SCREEN_IDLE)
				break;
		 if(time>LM75t.waitime)
		 {
				TM_USART_DMA_Send(USART1, (uint8_t *)LM75_tx_buffer,LM75_parpare_buffer());
			//set timer
				LM75t.timeout=time+LM75_TIMEOUT;//���ó�ʱʱ��
				LM75t.mission_state=SCREEN_DIS_POLLOING;
		 }
			break;
		
		case SCREEN_DIS_POLLOING:
			if(time>LM75t.timeout){//��Ƭ�����ͳ�ȥ��û���յ�Ӧ��
				LM75t.retry_count++;
				
				if(LM75t.retry_count>LM75_MAX_RETRY){
					LM75t.mission_state=SCREEN_DIS_FAILED;
					break;
				}
				LM75t.mission_state=SCREEN_DIS_PENDING;//����5�ξ������״̬��ѯ�Ƿ�ʱ5��
				break;
			}
				
			if(TM_USART_Gets(USART1,LM75_rx_buffer,5)){
				if(LM75_rx_buffer[0]==LM75_ACK){
						LM75t.index++;
						LM75t.retry_count=0;
					  LM75t.waitime=time+LM75WAITIME;
						LM75t.mission_state=SCREEN_DIS_QUERY;
				}
			}
				if(LM75t.index>=3){
					LM75t.mission_state=SCREEN_DIS_SUCCESS;
				}
					break;
				
		case SCREEN_DIS_FAILED:
			LM75t.index=0;
			LM75t.mission_state=SCREEN_DIS_IDLE;
			LM75t.retry_count=0;
		  LM75t.periodtime=time+LM75PRED_TIME;
			break;
		
		case SCREEN_DIS_SUCCESS:
				LM75t.index=0;
				LM75t.mission_state=SCREEN_DIS_IDLE;
				LM75t.retry_count=0;
		    LM75t.periodtime=time+LM75PRED_TIME;
			break;
		
		default:
				LM75t.mission_state=SCREEN_DIS_IDLE;
		break;
			}
	}
//********************************************************�¶���ʾ���������*************************************//

u8 LM75_parpare_buffer(){    //�޸���ɫ���¶�ֵ�����ͻ���
		u8 length;
		if(LM75t.index<3){
			 if(LM75t.index==0)//�¶ȵȼ�
			 {
				 length=10;
			   sprintf(LM75_tx_buffer,"rank.val=%d",LM75t.rank);
				 LM75_tx_buffer[length++]=0xFF;
         LM75_tx_buffer[length++]=0xFF;
         LM75_tx_buffer[length++]=0xFF;
			 }
			 if(LM75t.index==1)//������ֵ
			 {
				 if(LM75t.Integred>9)
				 length=9;
				 if(LM75t.Integred<10)
					 length=8;
			   sprintf(LM75_tx_buffer,"n0.val=%d",LM75t.Integred);
				 LM75_tx_buffer[length++]=0xFF;
         LM75_tx_buffer[length++]=0xFF;
         LM75_tx_buffer[length++]=0xFF;
			 }
			 if(LM75t.index==2)//С����ֵ 
			 {
				 length=8;
			   sprintf(LM75_tx_buffer,"n7.val=%d",LM75t.decide);
				 LM75_tx_buffer[length++]=0xFF;
         LM75_tx_buffer[length++]=0xFF;
         LM75_tx_buffer[length++]=0xFF;
			 }
		}
		return length;
}


void reser_screen(void){
	       u8 length;
				 length=6;
			   sprintf(LM75_tx_buffer,"page 0");
				 LM75_tx_buffer[length++]=0xFF;   //FF FF FF Ϊ������
         LM75_tx_buffer[length++]=0xFF;
         LM75_tx_buffer[length++]=0xFF;
	       TM_USART_DMA_Send(USART1, (uint8_t *)LM75_tx_buffer,length);//���ڷ��ͷ�������
        }

void lm75a_mission_polling(void){
	int16_t temp_int;
	char temp_frame[3];
	
		switch(	lm75_status){
			case LM75_IDLE:
				break;
			
			case	LM75_PENGDING:
				PRINTF("temp is %d \r\n",lm35_t.temp);
				if(lm35_t.temp>550)
				{
					
					lm75_status=LM75_FALIED;
				}else{
					lm75_status=LM75_SUCCESS;
				}
				
				break;
				
			case LM75_SUCCESS:
				temp_int=lm35_t.temp;			//25.2 >> 252
				
				temp_frame[0]=0x80;
				temp_frame[1]=temp_int>>8;
				temp_frame[2]=temp_int>>0;
				mission_state[TEMP_QUERY]=0x80;
				one_can_frame_send((u8 *)temp_frame,3,TEMP_QUERY);
			
				temp_int=b3470_get_temperature_offset(B3470_C2);			//25.2 >> 252
				
				temp_frame[0]=0x80;
				temp_frame[1]=temp_int>>8;
				temp_frame[2]=temp_int>>0;
				mission_state[TEMP_QUERY]=0x80;
				one_can_frame_send((u8 *)temp_frame,3,0xAA);
				lm75_status=LM75_IDLE;
			break;
			
			case	LM75_FALIED:
				mission_failed_send(TEMP_QUERY);
				lm75_status=LM75_IDLE;
			break;
				
			default :
				break;
		}
	
}




