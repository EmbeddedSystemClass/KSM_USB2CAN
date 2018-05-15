#include "lm75a.h"	
#include "cooler.h"
#include "MSD_test.h"  
#include "BUZZER.h"
__IO uint16_t ADC1OscConver[ADC_SIZE];
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
static void Adc1IoConfig(void)    //�ڲ�����
{
    /*
    IO������
    */
    GPIO_InitTypeDef    GPIO_InitStructure;
    //��C��ʱ�ӣ�
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    //��λ����
    GPIO_StructInit(&GPIO_InitStructure);
    //����Ϊģ������ģʽ
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AN;
    //��������
    GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
    //����������Ӧ���ڼĴ���
    GPIO_Init(GPIOA,&GPIO_InitStructure);
}
/*
����:tim2��������ʼ��
*/
static void Tim2Config(void)
{
    /*
    tim����
    */
    TIM_TimeBaseInitTypeDef         TIM_TimeBaseInitStructure;
    //��TIMʱ��8
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    //��ʼ�����ýṹ,�����������ø���
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
    //��APB1ʱ�Ӳ���Ƶ,��system_stm32f4xx.c��֪
    //APB1=AHB/4=SYSCLK/4=168M/4=42M,��TIM3=APB1*2=84M
    TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    //��Ƶֵ=168M/(Prescaler+1)/2=0.5us
    TIM_TimeBaseInitStructure.TIM_Prescaler=42-1;
    //���ʱ��ȷ��
    TIM_TimeBaseInitStructure.TIM_Period=44;
    //���ϼ�����ʽ
    TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	
    //����������Ӧ���ڶ�ʱ��
    TIM_TimeBaseInit(TIM8,&TIM_TimeBaseInitStructure);
    //���ݸ�����Ϊ����Դ
    TIM_SelectOutputTrigger(TIM8,TIM_TRGOSource_Update);
}
/*
����:ADC1��ʾ������������
��ע:ʹ��tim2������
*/
void Adc1OscConfig(void)
{
     
    /*
    dma����
    */
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;
    //����MDA2ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    //��ʼ�����Ĵ�������
    DMA_DeInit(DMA2_Stream0);
    DMA_StructInit( &DMA_InitStructure);
    //ѡȡDMAͨ��0,������0
    DMA_InitStructure.DMA_Channel=DMA_Channel_0;
	
    //���ݴ����ַ->ADC����ַ����DR�Ĵ���ƫ�Ƶ�ַ
    DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)ADC1_DR_Address;
    //���������ַ
    DMA_InitStructure.DMA_Memory0BaseAddr=(uint32_t)ADC1OscConver;
    //���ݴ��ͷ���Ϊ���赽SRAM
    DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralToMemory;
    //���ݻ�����1
    DMA_InitStructure.DMA_BufferSize=ADC_SIZE;
    //�����ַ�̶�
    DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
    //�ڴ��ַ����
    DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Enable;
    //��������Ϊ����
    DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;//DMA_PeripheralDataSize_Byte;//DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;//DMA_MemoryDataSize_Byte;//DMA_MemoryDataSize_HalfWord;
    //ѭ������
    DMA_InitStructure.DMA_Mode=DMA_Mode_Circular;
    //�����ȼ�
    DMA_InitStructure.DMA_Priority=DMA_Priority_High;
    //��ʹ��FIFOģʽ
    DMA_InitStructure.DMA_FIFOMode=DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold=DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst=DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst=DMA_PeripheralBurst_Single;
     
//  DMA_DoubleBufferModeCmd(DMA2_Stream0, DISABLE);
    //����������Ӧ����DMA2��ͨ��0��������0
    DMA_Init(DMA2_Stream0,&DMA_InitStructure);
    //ʹ��DMA
    DMA_Cmd(DMA2_Stream0, ENABLE);
    //ѡ��DMA2ͨ��������0
    NVIC_InitStructure.NVIC_IRQChannel=DMA2_Stream0_IRQn;//DMA2_Stream0_IRQHandler;
    //��ռʽ���ȼ�Ϊ0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
    //��Ӧʽ���ȼ�Ϊ12
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=4;
    //ͨ��ʹ��
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
    //����������Ӧ����NVIC
    NVIC_Init(&NVIC_InitStructure);
    //ʹ��DMA��������ж�
   // DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
     
    /*
    ����:ADC����
    */
    ADC_InitTypeDef ADC_InitStructure;
    //����ADC1ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    //12λת������
    ADC_InitStructure.ADC_Resolution=ADC_Resolution_12b;//ADC_Resolution_8b;//ADC_Resolution_12b; 
    //ʹ�õ�ͨ��ת��ģʽ
    ADC_InitStructure.ADC_ScanConvMode=DISABLE;
    //��ʹ�ö��ת��ģʽ
    ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;//ENABLE;//DISABLE;
    //ʹ���ⲿ�����ش���ģʽ
    ADC_InitStructure.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_Rising;//ADC_ExternalTrigConvEdge_Rising;//ADC_ExternalTrigConvEdge_None;
    //TIM3�������
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;//ADC_ExternalTrigConv_T1_CC1;
    //ѡ���Ҷ��뷽ʽ
    ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
    //ѡ��ת��ͨ����Ϊ1��
    ADC_InitStructure.ADC_NbrOfConversion=1;
    //����������Ӧ���ڶ�Ӧ�Ĵ���
    ADC_Init(ADC1,&ADC_InitStructure);
    //ʹ��ADC��DMAģʽ�µ�����ת��
    ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);
    //ʹ��ADC��DMAģʽ
    ADC_DMACmd(ADC1,ENABLE);
    //����ADC1������(�ó��䵥�β���ʱ��Ϊ:(3+12)/21��0.7us)->12bit
    ADC_RegularChannelConfig(ADC1,ADC_Channel_2,1,ADC_SampleTime_480Cycles);//ADC_SampleTime_3Cycles/ADC_SampleTime_480Cycles
    //ʹ��ADC
    ADC_Cmd(ADC1,ENABLE);
}
 
/*
����:��ADC��ͬ��������
*/
static void AdcSenCommConfig(void)
{
    ADC_CommonInitTypeDef   ADC_CommonInitStructure;
    //ADCΪ����ģʽ
    ADC_CommonInitStructure.ADC_Mode=ADC_Mode_Independent;
	
    //ADCʱ��ΪAPB2��2��Ƶ->84/4=21MHZ(F407ADC��2.4-3.6V�����ѹ���������36M,�ȶ��ٶ�Ϊ30M)
    ADC_CommonInitStructure.ADC_Prescaler=ADC_Prescaler_Div8;
	
    //ʹ��ADC��DMA����
    ADC_CommonInitStructure.ADC_DMAAccessMode=ADC_DMAAccessMode_2;
	
    //���β����ļ��ʱ��Ϊ5��ADCʱ������(5/21��0.24us)
    ADC_CommonInitStructure.ADC_TwoSamplingDelay=ADC_TwoSamplingDelay_5Cycles;
    //����������Ӧ���ڶ�Ӧ�ļĴ���
    ADC_CommonInit(&ADC_CommonInitStructure);
}
/*
����:��ʼADC1ʾ��������
*/
void StartAdc1OscSam(void)
{
 TIM_Cmd(TIM8, ENABLE);
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
}
/*
����:ֹͣADC1ʾ��������
*/
void StopAdc1OscSam(void)
{
 TIM_Cmd(TIM8, DISABLE);
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,DISABLE);
}

u8 lm75a_init(){

	Adc1IoConfig();
	AdcSenCommConfig();
	Tim2Config();
	Adc1OscConfig();
	
	return 0;
}

	
void lm75a_temp_read_polling(void){
//	u32 temp_val;
	switch(lm35_t.mission_state){          //���� ����״̬��ִ��
		
		case LM35_READ_IDLE:
			if(time>lm35_t.waitime){
			lm35_t.mission_state=LM35_READ_START;
				lm35_t.times=0;
				lm35_t.temp_all=0;
				//StartAdc1OscSam();
			}
			break;
			
		case LM35_READ_START:                   //��ʼ��ȡ�¶�
		/*	if(motor.running_state!=M_IDLE)
				break;
		*/
			lm35_t.temp_buffer[lm35_t.times]=Get_Adc_Average(ADC_Channel_2,100);      
			lm35_t.times++;                 //��1                                                          
			if(lm35_t.times>4)
					lm35_t.mission_state=LM35_READ_FINISH;
				
			break;
		
		case LM35_READ_FINISH:                   //�¶ȶ����Ĵ���
			
		for(u8 i=0;i<lm35_t.times;i++){
			//lm35_t.temp+=lm35_t.temp_buffer[i];
			lm35_t.temp_all+=lm35_t.temp_buffer[i];
		}
		
		lm35_t.temp_real=lm35_t.temp_all/lm35_t.times-3;  //�����¶���ƽ��  -3 ��Ϊ�˽������        temp_real ����ʵ�¶�
		
			//�����¶���������Χ�ڽ���
	  lm35_t.temp=temp_faded(lm35_t.temp_real,150);   //********�����¶Ƚ���         temp  �ǵ��ý�������ʾ�¶�	

/*		if(75<lm35_t.temp_real<=80)
			 lm35_t.temp-=5;
  	else if(80<lm35_t.temp_real<90)
			lm35_t.temp-=10;
*/
		
		lm35_t.mission_state=LM35_READ_IDLE;
		lm35_t.waitime=time+10000;
	
		
		if(lm35_t.temp_real<70){             //�����¶�ѡ��  ������ ����״̬
		lm35_t.temp_alarm_state=1;
		}
		
		if(lm35_t.temp_alarm_state==1)
		if(lm35_t.temp_real>150){
			buzzer_mission(MODE_THREE);
			lm35_t.temp_alarm_state=0;
		}
		
		if(time>lm35_t.close_inter_fan_time && lm35_t.close_inter_fan_enable)   //�ڲ����ȹرձ�־ ����  �� ʱ�����60S   �ر��ڲ�����
			close_inter_fan();
		
		if(lm35_t.cooler_function){    //������������
			if(lm35_t.temp_real<ZL_WD_L){
				cooler_off();
				lm35_t.close_inter_fan_time=time+60000;  //��ʱ60S
				lm35_t.close_inter_fan_enable =1;        //�ر��ڲ����ȱ�־����
				//����ʱ����ʱ�رշ���
				
			}
			if(lm35_t.temp_real>ZL_WD_H){
				cooler_on();
				lm35_t.close_inter_fan_enable=0;     //�ر��ڲ����ȱ�־Ϊ���ɹ�
			}
		}
		break;
	}
	
}


u16 temp_faded(u16 temp , u16 max_temp){   //�¶Ƚ�����򣨲�ѯ�¶�ʱ���յ���ʵ�¶ȣ�����¶ȣ�
		
		if(temp>=max_temp){           //�������15�� ��־Ϊ0����������¶�
			gobal_temp =temp;
			gobal_temp_flag=0;
			return gobal_temp;
		}
  
		
		
		//С��15��ʱ��
		if(!gobal_temp_flag){          //��ȡ�¶�ֵ  gobal_temp_flagĬ��Ϊ0
			gobal_temp_flag=1;
			gobal_temp = temp;
		}
		
		
		if(gobal_temp_flag){             //������¶���ȣ����䣬������0.1�����£�����б仯��0.2Ϊ��λ�仯
			if(gobal_temp == temp)
				return gobal_temp;
			
			if((gobal_temp - temp == 1) || (temp - gobal_temp == 1)){
				gobal_temp = temp;
				return gobal_temp;
			}
			
			if(gobal_temp < temp){
				gobal_temp += 2;
				return gobal_temp;
			}	
			
			if(gobal_temp > temp){
				gobal_temp -= 2;
				return gobal_temp;
			}			
		}
   return temp;
}


// *************************** �¶���ʾ������***************************//
void LM75_mission(void){
	float temp;
	u8 length=10;
	switch(LM75t.mission_state){
		
		case LM75T_IDLE:
			if(usart_state==0)
				 break;
					
			if(time>=LM75t.periodtime && sut.mission_state == SCREEN_IDLE)    //��ʾ�������ж�
					LM75t.mission_state=LM75T_PENDING;	
			break;
					
			
			
			
			
		case LM75_START:
			
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

				 
					LM75t.mission_state=LM75_START_CHECK;  //����Ƿ�ʼ����
					LM75t.timeout=time+200;
				//LM75t.mission_state=LM75T_IDLE;
		break;
		
		
				
				
				
		case  LM75_START_CHECK:     //���״̬
					if(sut.mission_state!=SCREEN_IDLE)
							break;
				
					//������ǿ��е� ���ٴγ��� ���0.2S
				if(time>LM75t.timeout){
					LM75t.retry_count++;
					LM75t.timeout=time+200;
					
					if(LM75t.retry_count>3){
						mission_failed_send(TEMP_DISPLAY);  //���Դ�������3�Σ���ʧ��
						LM75t.mission_state=LM75T_IDLE;
						LM75t.retry_count=0;
					}
					
							LM75t.mission_state=LM75_START;
					break;
				}
				
				if(TM_USART_Gets(USART1,LM75_rx_buffer,5)){//����յ����ص�ȷ��֡������ʾ�ɹ�
					if(LM75_rx_buffer[0]==LM75_ACK){
						LM75t.mission_state=LM75T_IDLE;
						usart_state=1;              //����������
						mission_success_send(TEMP_DISPLAY);
				}
			}
		
		break;
		
			
			
		case LM75T_PENDING:
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
		      LM75t.mission_state=LM75T_QUERY;
					LM75t.waitime=time+LM75WAITIME;
					LM75t.index=1;
			break;
					
					
		case LM75T_QUERY:  //��ʱ�����¶�
			if(sut.mission_state != SCREEN_IDLE)
				break;
		 if(time>LM75t.waitime)
		 {
				TM_USART_DMA_Send(USART1, (uint8_t *)LM75_tx_buffer,LM75_parpare_buffer());
			//set timer
				LM75t.timeout=time+LM75_TIMEOUT;//���ó�ʱʱ��
				LM75t.mission_state=LM75T_POLLOING;
		 }
			break;
		
		case LM75T_POLLOING:
			if(time>LM75t.timeout){//��Ƭ�����ͳ�ȥ��û���յ�Ӧ��
				LM75t.retry_count++;
				
				if(LM75t.retry_count>LM75_MAX_RETRY){
					LM75t.mission_state=LM75T_FAILED;
					break;
				}
				LM75t.mission_state=LM75T_PENDING;//����5�ξ������״̬��ѯ�Ƿ�ʱ5��
				break;
			}
				
			if(TM_USART_Gets(USART1,LM75_rx_buffer,5)){
				if(LM75_rx_buffer[0]==LM75_ACK){
						LM75t.index++;
						LM75t.retry_count=0;
					  LM75t.waitime=time+LM75WAITIME;
						LM75t.mission_state=LM75T_QUERY;
				}
			}
				if(LM75t.index>=3){
					LM75t.mission_state=LM75T_SUCCESS;
				}
					break;
				
		case LM75T_FAILED:
			LM75t.index=0;
			LM75t.mission_state=LM75T_IDLE;
			LM75t.retry_count=0;
		  LM75t.periodtime=time+LM75PRED_TIME;
			break;
		
		case LM75T_SUCCESS:
				LM75t.index=0;
				LM75t.mission_state=LM75T_IDLE;
				LM75t.retry_count=0;
		    LM75t.periodtime=time+LM75PRED_TIME;
			break;
		
		default:
				LM75t.mission_state=LM75T_IDLE;
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
	u16 temp_int;
	char temp_frame[3];
	
		switch(	lm75_status){
			case LM75_IDLE:
				break;
			
			case	LM75_PENGDING:
				
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


void bubble_sort_better(__IO u16 a[],u16 n)  //����ʲô��  ��ADC�й�
{
    for(u16 i=0; i<n-1; i++)
    {
        u16 isSorted = 0;
        for(u16 j=0; j<n-1-i; j++)
        {
            if(a[j] > a[j+1])
            {
                isSorted = 1;
                u16 temp = a[j];
                a[j] = a[j+1];
                a[j+1]=temp;
            }
        }
        if(isSorted) break; 
    }
}

//


void DMA2_Stream0_IRQHandler(void)     //MDA2 �жϴ���
{	
	
	
	 	if(DMA_GetFlagStatus(DMA2_Stream0,DMA_IT_TC)==SET)  
   {   
		 StopAdc1OscSam(); //�ر�
		 lm35_t.mission_state=LM35_READ_FINISH;		 
		DMA_ClearFlag(DMA2_Stream0,DMA_IT_TC);	
}
	
} 


