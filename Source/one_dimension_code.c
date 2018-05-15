#include "one_dimension_code.h"
char USART_Buffer2[200];
char one_dimension_code_state=0;
odc_t odc;
extern u32 time;

void one_dimension_code_init(){          //��ά�벿�ֳ�ʼ�����񣬸��ֳ�ʼ������ʼ���˿ڣ�ͨ���ߣ������ʣ�ʱ��ʲô��
	TM_GPIO_Init(GPIOD, GPIO_Pin_7, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_High); //��ʼ���˿�
	TM_USART_Init(USART2, TM_USART_PinsPack_2, 9600);  //
	TM_USART_DMA_Init(USART2);
	TM_GPIO_SetPinHigh(GPIOD, GPIO_Pin_7);
	odc.waittime=0;
	odc.timeout=0;
}

void one_dimension_code_start(){    //��ɨ��ͷ PD7��0
	//TM_GPIO_TogglePinValue(GPIOD, GPIO_Pin_7);
	TM_GPIO_SetPinLow(GPIOD, GPIO_Pin_7);
	
}


uint16_t one_dimension_code_read(){                        //   ����ά��
	return TM_USART_Gets(USART2, odc.buf, sizeof(odc.buf));
}

void one_dimension_code_mission_polling(){  //one_dimension_code ��ά������
	switch(odc.running_state){
		case ODC_IDLE:            //0x00   �޶���
		break;
		
		case ODC_ENABLE:      //0x01      ʹ��
			TM_GPIO_SetPinLow(GPIOD, GPIO_Pin_7);   // PD7 �õ�     ��
			odc.running_state=ODC_DATAREAD;        //״̬����Ϊ������
			odc.timeout=time+ODC_OUTTIME;          // ��ʱʱ��= ����ʱ��+���ʱ��750
		break;
		
		case ODC_DATAREAD:       //0x02    ������
			
			if(time>odc.timeout)                      //���ʵ��ʱ�䳬ʱ��
				odc.running_state=ODC_TIMEOUT;          //����״̬����Ϊ    0x03 ��ʱ
				odc.length=one_dimension_code_read();   // ��������Ϊ ...
			if(odc.length){                            // �߼��ж�   ������� Ϊ��0 ��Ϊ�� ���˴��� ������ȷ�0
				TM_GPIO_SetPinHigh(GPIOD, GPIO_Pin_7);   // PD7 �ø�     ��
				odc.running_state=ODC_SUCCESS;           // ״̬����Ϊ�ɹ�
				buzzer_mission(MODE_TWO);                 // ����������  2  
			}
			break;
			
		case ODC_TIMEOUT:         //0x03   ��ʱ
			TM_GPIO_SetPinHigh(GPIOD, GPIO_Pin_7);   // PD7 �ø� �ر�
			odc.timeoutCount++;                          //��ʱ����+1
			if(odc.timeoutCount>=ODC_MAX_RETRY){         //������������Դ���
					//mission_failed_send(ODC_UPLOAD);      //����ʧ�ܱ�־
				mission_success_send(ODC_UPLOAD);       //
					odc.running_state=ODC_IDLE;          //��������
					odc.timeoutCount=0;                  //��ʱ��������
					break;
				}
			odc.running_state=ODC_PENDDING;        //����״̬ ����
			odc.waittime=time+ODC_WAITTIME;
		break;
		
		case ODC_PENDDING:             //0x04   ����
			if(time>=odc.waittime)         // ��һ��ʱ���ʹ�� 
				odc.running_state=ENABLE;
			break;
			
		case ODC_SUCCESS:             //0x05    �ɹ�
			
			if((odc.length)==0){                  //û�յ����ݣ� ״̬����Ϊ��ʱ
			odc.running_state=ODC_TIMEOUT;
				break;
			}
			//odc.buf[odc.length]=0x0A;
			//odc.length=odc.length+1;
			odc.waittime=time+ODC_WAITTIME;
			
			can_mutil_frame_send((uint8_t *)odc.buf,odc.length,0xA0);
			mission_success_send(ODC_UPLOAD);   //���� 
			odc.running_state=ODC_IDLE;
			odc.length=0;
			odc.timeoutCount=0;           //֮ǰû����һ�䣬 ��ʱ���������ۻ�
		break;
		
	}
	
	
}

