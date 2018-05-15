/**
  ******************************************************************************
  * @file    MSD_test.c
  * @author  ��УԴ
  * @version V1.0
  * @date    2017-07-11
  * @brief   ��������������
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */ 
#include "MSD_test.h"  
#include "MicroStepDriver.h" 
#include "coder.h"
#include "main.h"
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_usart_dma.h"
#include "can.h"
#include "cooler.h"
#define WAITTIME 100
motor_t motor;
extern char USART_Buffer[100];

//�趨�����ʱʱ��
void motor_timer_set(){
motor.timeout=time+RESET_TIMEOUT;
}

//�������ʱ �����ʱ�ͷ���1 �������³�ʼ���������
 u8 motor_timeout_check(){
	 
	if(time>motor.timeout){
		srd.run_state=STOP;
		u8 buf[1]={motor.running_state};
		action_value_send_none_80((u8 *)buf,1,0xA1);
		motor.error_state=motor.running_state;
		motor.running_state=MOTOR_TIME_OUT;
		MSD_Init();
		return 1;
	}
		return 0;
}
 
/*
void motor_reset_new(void){
	
	switch(motor.running_state){
	
			case M_IDLE:
				break;
			case M_RESET_START:
				if(motor.waittime>time)
					break;
				MSD_Move(RESET_STEP,MOTOR_RESET_SPEED,MOTOR_RESET_SPEED,MOTOR_RESET_SPEED);
				motor_timer_set();
				motor.running_state=M_RESET_WAIT_STOP;
				break;
				//
			case M_RESET_WAIT_STOP:
				if(motor_timeout_check())
										break;
				//������Ƿ�����ֹͣ���������ֹͣ������ʧ��
			break;
			
				
			case M_RESET_POSITION:
			
					if(motor.waittime>time)
							break;
					if(coder_init_count==0){
							motor.running_state=MOTOR_TIME_OUT;
							break;
						}				
						coder_init_count=0;
						MSD_Move(RESET_OFFSET,MOTOR_RESET_SPEED,MOTOR_RESET_SPEED,MOTOR_RESET_SPEED);
						motor_timer_set();
						motor.running_state=M_WAITTING_CODERSTEP;
							break;
					
				case M_WAITTING_CODERSTEP:
					
									if(motor_timeout_check())
										break;
								
									if(srd.run_state!=STOP)
										break;
									
									motor.running_state=M_RESET_END;
									break;
					
				case M_RESET_END:
								//����λ��
								if(power_off_state){
									power_off_state=0;
									power_off();
								}
										srd.position=0;			
										action_success_send();
										motor_finish();
							
								break;
					
				case MOTOR_TIME_OUT:
					
									if(power_off_state){
										power_off_state=0;
										power_off();
									}
										action_failed_send();
										motor_finish();
									break;
					
					default:
						break;
	}
	
	
}

*/



//�����λ
void motor_reset(void){
			switch(motor.running_state){
				
				case M_IDLE:
					break;
				
				case M_RESET_START:
					//ֻ�п��е�ʱ�����������
					//����ʼ��ʱ��ʱ
					
				if(motor.waittime>time)
						break;
					//��λ�жϱ�־
				  if(coder_init_count==1)
						coder_init_count=0;
					MSD_Move(CIRCUL_STEP,MOTOR_SPEED,MOTOR_SPEED,MOTOR_HISPEED);
					motor_timer_set();
				
					motor.running_state=M_RESET_POSITION;
				break;
				
				case M_RESET_POSITION:
					
				if(motor_timeout_check())
						break;
				
				if(srd.run_state!=STOP){
					motor.waittime=time+MOTOR_WAITTIME;
					break;
					}
				
					if(motor.waittime>time)
						break;
					//���û�н�����жϣ���û��ʶ��λ�㣬
					if(coder_init_count==0){
						coder_init_count=2;
						motor.running_state=M_RESET_START;
						break;
					}
					if(coder_init_count==2){
						coder_init_count=0;
						motor.running_state=MOTOR_TIME_OUT;
						break;
					}
						/*sprintf(USART_Buffer, "%d %d \r\n",srd.position_to_move,coder_setp);
					TM_USART_DMA_Send(USART1, (uint8_t *)USART_Buffer,strlen(USART_Buffer));
					*/
					if(coder_setp>HALF_CIRCUL_STEP){
						MSD_Move(coder_setp-CIRCUL_STEP,MOTOR_SPEED,MOTOR_SPEED,MOTOR_HISPEED);
					}else{
						MSD_Move(coder_setp,MOTOR_SPEED,MOTOR_SPEED,MOTOR_HISPEED);
					}
					
					motor_timer_set();
					motor.running_state=M_WAITTING_CODERSTEP;
					
					//���ý��� ����
					break;
					
				case M_WAITTING_CODERSTEP:
					
					if(motor_timeout_check())
						break;
				
					if(srd.run_state!=STOP)
						break;
					
					motor.running_state=M_RESET_END;
					break;
					
				case M_RESET_END:
					//����λ��
				if(power_off_state){
					power_off_state=0;
					power_off();
				}
						srd.position=0;			
						action_success_send();
						motor_finish();
			
				break;
					
				case MOTOR_TIME_OUT:
					
				if(power_off_state){
					power_off_state=0;
					power_off();
				}
					action_failed_send();
					motor_finish();
				break;
					
					default:
						break;
					
				}
			
}



//�ƶ�����  ��ѯ
void motor_move_polling(void){
			switch(motor.running_state){
				case M_IDLE:
					break;
				case M_PENDDING:
					//start the motor
				if(motor.waittime>time)
						break;
					move_to_position(srd.position_to_move);
					//set timer	
					motor_finish();
					motor.running_state=M_RUNNING;
				break;
				case M_RUNNING:
					
					if(motor_timeout_check())
						break;
					
					if(srd.run_state!=STOP)
						break;
					motor.running_state=M_FINISH;
					break;
					
				case M_FINISH:
				
					action_success_send();
					motor_finish();		
					break;
					//send can mesage to USB;
					// close the moter
				default:
					break;
				}
			
}



//�ƶ� ��ָ��λ��
u8 move_to_position(u16 position){
	int steps_to_move = position-srd.position;
	if(steps_to_move<-HALF_CIRCUL_STEP)
		steps_to_move=steps_to_move+CIRCUL_STEP;
	if(steps_to_move>HALF_CIRCUL_STEP)
		steps_to_move=steps_to_move-CIRCUL_STEP;
	MSD_Move(steps_to_move,MOTOR_MOVE_ACC_SPEED,MOTOR_MOVE_DEL_SPEED,MOTOR_MOVE_MAX_SPEED);
	motor_timer_set();
	return 0;
}


//��ҡ
void motor_shocking(void){

	switch(motor.running_state){
			case M_IDLE:
					break;
			
			case M_SHOCK_PENDING:
				if(motor.waittime>time)
						break;
				motor.running_state=M_Positive;
				motor_timer_set();
				break;
				
			case M_Positive:
				if(motor_timeout_check())
						break;
				
				if(srd.run_state!=STOP){
					motor.waittime=time+MOTOR_SHOCKING_WAIT_TIME;
					break;
				}
				
				if(motor.waittime>time)
						break;
				
				MSD_Move(10000,SHOCKING_SPEED_UP,SHOCKING_SPEED_UP,SHOCKING_MAX_SPEED);
				motor_timer_set();
					
				motor.running_state=M_Reverse;
				
				break;
				
			case M_Reverse:
				
				if(motor_timeout_check())
						break;
				
				if(srd.run_state!=STOP){
					motor.waittime=time+MOTOR_SHOCKING_WAIT_TIME;
					break;
				}

				
				if(motor.waittime>time)
						break;				
					
					MSD_Move(-10000,SHOCKING_SPEED_UP,SHOCKING_SPEED_UP,SHOCKING_MAX_SPEED);
				
					motor_timer_set();
					motor.running_state=M_Positive;
					break;
				
			case M_PR_END:

				/*
				if(motor_timeout_check())
						break;
				*/
			
				if(srd.run_state!=STOP){
					motor.waittime=time+MOTOR_WAITTIME;
					break;
				}
				
				if(motor.waittime>time)
						break;
				motor_timer_set();
				motor.running_state=M_RESET_START;
					break;
				
			default:
				break;
	}


}


//���״̬����
void motor_maintain_polling(void){

	switch(motor.running_state){
			case M_IDLE:
					break;
			case M_MAINTAIN_PENDING:
				if(motor.waittime>time)
						break;
				if(motor.maintain_dir==0x00)
				motor.running_state=M_MAINTAIN_POSITIVE;
				if(motor.maintain_dir==0x01)
				motor.running_state=M_MAINTAIN_REVERSE;				
				motor_timer_set();
				break;
			case M_MAINTAIN_POSITIVE:
				if(motor_timeout_check())
						break;
				
				if(srd.run_state!=STOP){
					motor.waittime=time+MOTOR_WAITTIME;
					break;
				}
				
				if(motor.waittime>time)
						break;
				
				MSD_Move(20000,SHOCKING_SPEED_UP,SHOCKING_SPEED_UP,MAINTAIN_MAX_SPEED);
				motor_timer_set();
					
				motor.running_state=M_MAINTAIN_POSITIVE;
				
				break;
			case M_MAINTAIN_REVERSE:
				
				if(motor_timeout_check())
						break;
				
				if(srd.run_state!=STOP){
					motor.waittime=time+MOTOR_WAITTIME;
					break;
				}

				
				if(motor.waittime>time)
						break;				
					
					MSD_Move(-20000,SHOCKING_SPEED_UP,SHOCKING_SPEED_UP,MAINTAIN_MAX_SPEED);
				
					motor_timer_set();
					motor.running_state=M_MAINTAIN_REVERSE;
					break;
				
			default:
				break;
	}


}
	


//�Ƕȶ��壬ת��Ϊ������
float angle(unsigned int nn,unsigned int mm,unsigned int pp)  //nnΪ�Լ����ţ�mm�Լ���λ�úţ�pp�Լ��ܺ�
{
	float angle;
	switch(nn)
	{
		case 1:
			if(pp==3)
			{
				angle=(float)((360-61.71)-(mm-1)*15);
			}
			if(pp==2)
			{
				angle=(float)((360-67.51)-(mm-1)*15);
			}
			if(pp==1)
			{
				angle=(float)((360-71.29)-(mm-1)*15);
			}
			break;
		case 2:
			if(pp==1||pp==2||pp==3)
			{
				angle=(float)((360-90)-(mm-1)*15);
			}
			break;
		case 3:
			if(pp==3)
			{
				angle=(float)((360-118.29)-(mm-1)*15);
			}
			if(pp==2)
			{
				angle=(float)((360-112.49)-(mm-1)*15);
			}
			if(pp==1)
			{
				angle=(float)((360-108.71)-(mm-1)*15);
			}
			break;
		default:break;
	}
	
	angle=360-angle;
	return angle*CIRCUL_STEP/360;
}



//�ƶ� ����        ���ص����Ҫ�ߵ�������
u16 step_to_move_calc(unsigned int nn,unsigned int mm,unsigned int pp)//nnΪ�Լ����ţ�mm�Լ���λ�úţ�pp�Լ��ܺ�
{
	float angle;
	u16 position;
	switch(nn)
	{
		case 1:
			if(pp==3)             //1���� 3���Լ���
			{
				angle=61.71+(mm-1)*15;
				//angle=(float)((360-61.71)-(mm-1)*15);
			}
			if(pp==2)           //1���� 2���Լ���
			{
				angle=67.51+(mm-1)*15;
				
				//angle=(float)((360-67.51)-(mm-1)*15);
			}
			if(pp==1)        //1���� 1���Լ���
			{
				angle=71.29+(mm-1)*15;
				//angle=(float)((360-71.29)-(mm-1)*15);
			}
			break;
		case 2:
			if(pp==1||pp==2||pp==3)           //2���� 1��2,3�����Լ���
			{
				angle=90+(mm-1)*15;
				//angle=(float)((360-90)-(mm-1)*15);
			}
			break;
		case 3:
			if(pp==3)                    //3���� 3���Լ���
			{
				angle=118.29+(mm-1)*15;
				//angle=(float)((360-118.29)-(mm-1)*15);
			}
			if(pp==2)              //3���� 2���Լ���
			{
				angle=112.49+(mm-1)*15;
				//angle=(float)((360-112.49)-(mm-1)*15);
			}
			if(pp==1)                  //3���� 1���Լ���
			{
				angle=108.71+(mm-1)*15;
				//angle=(float)((360-108.71)-(mm-1)*15);
			}
			break;
		default:break;
	}
	position=(u16)(angle*ANGLE_CALC_STEP);   //�Ƕȳ��� ÿ�� ��Ӧ��������=���Ҫ�ߵ�������

		position=position%CIRCUL_STEP;    //  ʵ�ʵ��Ҫ�ߵ�������=Ҫ�ߵ������� % һȦ�������� ����ģ��  
		
	return position;
}


u16 MM_position(u8 mm){
	short int position =(mm-1)*15*CIRCUL_STEP/360;
	position+=motor.offset[1];
	
	if(position<0)
		position+=CIRCUL_STEP;
	
	if(position>CIRCUL_STEP)
		position-=CIRCUL_STEP;
	
	return position;
}

void motor_finish(void){
	motor.waittime=time+MOTOR_WAITTIME;
	motor.running_state=M_IDLE;		
}


//Ҫ�仯��λ��������
u16 motor_inject_offset_position(unsigned int nn,unsigned int mm,unsigned int pp){  //nnΪ�Լ����ţ�mm�Լ���λ�úţ�pp�Լ��ܺ�
	float position;
	position=step_to_move_calc(nn,mm,pp);
	
	switch(nn)
	{
		case 1:
			if(pp==1)
			{
				position+=motor.offset[2];
			}
			if(pp==2)
			{
				position+=motor.offset[3];
			}
			if(pp==3)
			{
				position+=motor.offset[4];
			}
			break;
		case 2:
			if(pp==1)
			{
				position+=motor.offset[5];
			}
			if(pp==2)
			{
				position+=motor.offset[6];
			}
			if(pp==3)
			{
				position+=motor.offset[7];
			}
			break;
			
		case 3:
			if(pp==1)
			{
				position+=motor.offset[8];
			}
			if(pp==2)
			{
				position+=motor.offset[9];
			}
			if(pp==3)
			{
				position+=motor.offset[10];
			}
			break;
		default:break;
	}
	
		if(position<0)
		position+=CIRCUL_STEP;
		
	if(position>CIRCUL_STEP)
		position-=CIRCUL_STEP;
		
	return (u16)position;
}


/*********************************************END OF FILE**********************/
