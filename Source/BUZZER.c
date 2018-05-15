#include "BUZZER.h"

extern u32 time;
buzzer_t buzzer;     //���� �ṹ����� buzzer    �����buzzer_t�Ѿ������������������ָ�������ͣ��ṹ�壩

void BUZZER_Init(void)   //��������ʼ��

{
	  GPIO_InitTypeDef GPIO_InitStructure;
	  RCC_AHB1PeriphClockCmd(BUZZER_GPIO_CLK, ENABLE);//ʹ��GPIOD��ʱ��
    GPIO_InitStructure.GPIO_Pin =  BUZZER_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT,&GPIO_InitStructure);
    GPIO_ResetBits(BUZZER_PORT,BUZZER_PIN);
}



void BUZZER_mission_polling()
{
	
	switch(buzzer.running_state){     //���ݹ���״̬  ѡ��
					case BUZZERIDLE:
					break;			
							
					case BUZZERPENDDING:     //  0x01   ����״̬
						if(buzzer.current_mission==MODE_ONE){
							buzzer.uptime[0]=100;
							buzzer.length=1;
						}
						if(buzzer.current_mission==MODE_TWO){
							buzzer.uptime[0]=50;
							buzzer.length=1;
						}
						if(buzzer.current_mission==MODE_THREE){
							buzzer.uptime[0]=1000;
							buzzer.uptime[1]=1000;
							buzzer.uptime[2]=1000;
							buzzer.length=3;
						}						
						
						buzzer.index=0;
						buzzer.running_state=UP;
						buzzer.waittime=time+buzzer.uptime[buzzer.index++];
						GPIO_SetBits(BUZZER_PORT,BUZZER_PIN);
						break;			
					case UP:
						if(time>buzzer.waittime){
							GPIO_ResetBits(BUZZER_PORT,BUZZER_PIN);
							buzzer.waittime=time+DOWN_TIME;
							buzzer.running_state=DOWN;
						}
						break;				
					case DOWN:
						if(time>buzzer.waittime){				
							buzzer.waittime=time+buzzer.uptime[buzzer.index++];
							if(buzzer.index>buzzer.length){
								buzzer.running_state=BUZZERIDLE;
								GPIO_ResetBits(BUZZER_PORT,BUZZER_PIN);
								break;
							}
							GPIO_SetBits(BUZZER_PORT,BUZZER_PIN);
							buzzer.running_state=UP;
						}
						break;
					}	
}

void buzzer_mission(u8 mission){
	
	buzzer.running_state=BUZZERPENDDING;   // 0x01   ���ù���״̬   �������������ѯ
	buzzer.current_mission=mission;       //mission=  �������ط���ȡ
}
