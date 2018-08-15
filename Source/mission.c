#include "mission.h"

#define DEBUG 0
#if DEBUG
#include "usart.h"
#define PRINTF(...)   printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

extern  __IO uint8_t USB_StatusDataSended;
extern  uint32_t USB_ReceivedCount;
extern  uint8_t USB_Tx_Buffer[];   //������
extern  uint8_t USB_Rx_Buffer[];   //�ջ���
extern __IO uint8_t DeviceConfigured;
u32 usb_waittime;
u8 motor_tims=0;
                                                           //ִ������
u8 usart_state __attribute__((at(0x10000018)));
char version[7];

//��ʼ������
void timer_reset(void){
	time=0;
	motor.timeout=0;
	motor.waittime=0;
	motor.dia_time =0;
	lm35_t.waitime=0;
	lm35_t.close_inter_fan_time=0;
	lm35_t.times=0;
	lm35_t.pwm_time=0;
	
	sut.timeout=0;
	sut.waitime=0;
	
	buzzer.waittime=0;
	buzzer.ranktime=0;
	LM75t.periodtime=0;  
	odc.timeout=0;
	odc.waittime=0;
	
	gobal_temp_flag=0;
	
	lmt.can_rx_time=0;
	lmt.usb_rx_time=0;
	lmt.usb_polling_time=0;
	lmt.usb_rx_time=0;
	usb_waittime=0;
	for(u8 i=0;i<255;i++)
		mission_state[i]=0;
}


//�������ѯ
void mission_polling(void){
	iwdg_t *wdg = &_iwdg;
    switch(USB_Rx_Buffer[7]){   //�������ڵ�����ţ�  ��7���ֽ��������
			
				case MOTOR_RESET:            //10 09  �Լ��̸�λ
					if(power_off_state)
						break;
					motor.current_mission=MOTOR_RESET;
					motor_timer_set();
					motor.running_state=M_RESET_START; 
					//wdg->wdg_motor_mission_backup(wdg,&motor);
					if(wdg->is_iwdg_set)
						wdg->wdg_flag_clear(wdg);
					motor_tims=0;
				break;
				
				case MOTOR_INJECTION:    // 10 0B   �ƶ�����ע
					if(power_off_state)
						break;
					
					srd.position_to_move=motor_inject_offset_position(USB_Rx_Buffer[8],USB_Rx_Buffer[9],USB_Rx_Buffer[10]);
					motor.running_state=M_PENDDING;
					motor.current_mission=MOTOR_INJECTION;
					motor_tims++;
					if(motor_tims>10){
						
					wdg->wdg_flag_set(wdg);
					motor_tims=0;
					}
					wdg->wdg_motor_mission_backup(wdg,&motor);
					if(wdg->is_iwdg_set)
					{
						wdg->wdg_motor_mission_set(wdg,&motor);
					}
					
					motor_timer_set();
				break;
				
				case MOTOR_SHOCKING_START:     //  10 10 ������ҡ
					if(power_off_state)
						break;
					motor.running_state=M_SHOCK_PENDING;
					motor.current_mission=MOTOR_SHOCKING_START;
					if(wdg->is_iwdg_set)
					{
						wdg->wdg_motor_mission_set(wdg,&motor);
					}
					action_success_send();
				break;
				
				case MOTOR_SHOCKING_STOP:           // 10 11   �ر���ҡ
					if(power_off_state)
						break;
					motor.running_state=M_PR_END;
					motor.current_mission=MOTOR_SHOCKING_STOP;
					if(wdg->is_iwdg_set)
					{
						wdg->wdg_motor_mission_set(wdg,&motor);
					}
					motor_tims=0;
					motor_timer_set();
				break;
				
				case MOTOR_INSTALL:       // 10 0A    ��װ�Լ�
					if(power_off_state)
						break;
						srd.position_to_move=MM_position(USB_Rx_Buffer[8]);
						motor.running_state=M_PENDDING;
						motor.current_mission=MOTOR_INSTALL;
						wdg->wdg_motor_mission_backup(wdg,&motor);
						if(wdg->is_iwdg_set)
						{
						wdg->wdg_motor_mission_set(wdg,&motor);
						}					
						motor_timer_set();
				break;
					
				case TEMP_QUERY:            //  10 0C  �¶Ȳ�ѯ
					lm75_status=LM75_PENGDING;
				break;
				
				case TIME_MODIFY:          // 10 12  �޸����� �ѽ��յ���ʱ�丳ֵ����ʾ��ʱ�仺��
					rct_t.year1=USB_Rx_Buffer[8]; 
					rct_t.year2=USB_Rx_Buffer[9];
					rct_t.buf[0]=USB_Rx_Buffer[10];
					rct_t.buf[1]=USB_Rx_Buffer[11];
					rct_t.buf[2]=USB_Rx_Buffer[12];
					rct_t.buf[3]=USB_Rx_Buffer[13];
					rct_t.buf[4]=USB_Rx_Buffer[14];
					sut.mission_state=SCREEN_PENDING;
					sut.current_mission=TIME_MODIFY;
				break;
				
				case MOTOR_DEBUG:          //  10 05     �޸ĵ������  ����
					PRINTF("MOTOR_DEBUG para %d  value%d\r\n",USB_Rx_Buffer[9]-1 , (USB_Rx_Buffer[12]<<8)|(USB_Rx_Buffer[13]<<0));
					fmt.buf[USB_Rx_Buffer[9]-1]=(USB_Rx_Buffer[12]<<8)|(USB_Rx_Buffer[13]<<0);
					fmt.current_mission=MOTOR_DEBUG;
					fmt.mission_state=MEMORY_WIRTE_SUCCEE;
					fmt.newdata=1;
				break;
				
				//һֱ
				
				case MOTOR_MATAIN:                      // 10 17  �������
					motor.maintain_dir=USB_Rx_Buffer[8];
					motor.running_state=M_MAINTAIN_PENDING;
					motor.current_mission=MOTOR_MATAIN;
				break;
				
				case FLASH_MISSION_WRITE:          // 10 07   д��
					fmt.current_mission=FLASH_MISSION_WRITE;
					fmt.mission_state=FLASH_WRITE;
					fmt.newdata=1;
					break;
				
				case FLASH_MISSION_READ:        //10 06   ��
					fmt.current_mission=FLASH_MISSION_READ;
						fmt.mission_state=FLASH_READ;
						fmt.command_index=USB_Rx_Buffer[9]-1;
				break;
				
				case COOLER_START:         // 10 0D   ��������
					
					  cooler_received_command=1;   //�յ�����������
						temp_control=1;
#if USE_LM35
						lm35_t.pwm_time=0;
						lm35_t.cooler_pwm_function =1;
#else
						lm35_t.cooler_function=1;				
#endif 
						cooler_on();
						mission_success_send(COOLER_START);
				break;
				
				case COOLER_STOP:               // 10 0E    �ر�����
							temp_control=0;
#if USE_LM35
							lm35_t.cooler_pwm_function = COOLER_OFF;				
#endif 
							lm35_t.cooler_function=0;

							cooler_off();
							mission_success_send(COOLER_STOP);
				break;
				
				case TEMP_DISPLAY:                  // 13  �¶���ʾ 
					if(usart_state==0)
					LM75t.mission_state=SCREEN_DIS_START;
					else
						mission_success_send(TEMP_DISPLAY);
			break;
				
				case TEMP_SHINING:             //14    �޸��¶ȱ�����ɫ
					mission_success_send(TEMP_SHINING);
				break;
				
				case POWER_ON:                       //15   ����Դ
				if(USB_Rx_Buffer[8]==0xA1){	
					power_off_state=1;          // 10 15 A1 �ص�
					TIM_Cmd(MSD_PULSE_TIM, DISABLE);
					MSD_PULSE_TIM->CCER &= ~(1<<12); //��ֹ���
					MSD_Init();
					power_off();
					timer_reset(); //�ػ�ʱ�����ò���
					mission_success_send(POWER_OFF);
					motor.running_state=M_IDLE;
					mission_success_send(POWER_OFF);
					power_satus=0;
					//���֮ǰ�յ����������� ���´�����
					if(cooler_received_command)
					{			
						temp_control=1;
						lm35_t.pwm_time=0;
						lm35_t.cooler_pwm_function =1;
						cooler_on();
						lm35_t.cooler_function = 1;
					}
					break;
				}
				
				if(USB_Rx_Buffer[8]==0xB2){     //10 15 B2 ����
					power_satus=1;
					power_off_state=0;
					power_on();
					//timer_reset();
					flash_init();
					mission_success_send(POWER_ON);
					break;
				}
				
				if(USB_ReceivedCount==9){
					mission_success_send(POWER_ON);	
					break;
				}
				
				if(USB_ReceivedCount==8){
					power_on();
					//timer_reset();
					mission_success_send(POWER_ON);				
				}
				
				break;
				
				case POWER_OFF:             //  16     �رյ�Դ
					power_off_state=1;        //��Դ�ر�־
					/*
					TIM_Cmd(MSD_PULSE_TIM, DISABLE);
					MSD_PULSE_TIM->CCER &= ~(1<<12); //��ֹ���
					power_off();
					*/
					motor.running_state=M_PR_END;
					motor.current_mission=MOTOR_SHOCKING_STOP;   //ֹͣ��ҡ
					mission_success_send(POWER_OFF);
					//power_off();
					//mission_success_send(POWER_OFF);
					break;
				
				case ODC_UPLOAD:         //10 0F �ϴ���ά��
					odc.running_state=ODC_PENDDING;
					odc.timeoutCount=0;
				break;
				
				case VERSION_UPLOAD:   //10 03 �ϴ��汾��
					sprintf(version,"1.77LA");
#if USE_LM35
					version[4]='L';
#else
					version[4]='B';
#endif 		
					action_value_send_none_80((u8 *)version,6,VERSION_UPLOAD);
				break;
				
				case STATE_QUERY:
					//version[0]=USB_Rx_Buffer[8];
					//�����ѯ�¶�ֱ�ӷ����¶�
					if(USB_Rx_Buffer[8]==TEMP_QUERY){
						lm75_status=LM75_SUCCESS;
						break;
					}
					version[0]=mission_state[USB_Rx_Buffer[8]];
					action_value_send_none_80((u8 *)version,1,USB_Rx_Buffer[8]);
				break;
					
				case LED1_ON_CMD:           //10 18������LED1������
					  switch(USB_Rx_Buffer[8]){
							case RED_LED_t:
								LED1_ON(RED_LED1);
						  	LED1_OFF(GREEN_LED1);
							
							break;
							case GREEN_LED_t:
								LED1_ON(GREEN_LED1);
								LED1_OFF(RED_LED1);
								
							break;
							
						}
						mission_success_send(LED1_ON_CMD);
				break;		
						
				case LED1_OFF_CMD:           //10 19������LED1������
				//	if(power_off_state)
					//	break;		
					  switch(USB_Rx_Buffer[8]){
							case RED_LED_t:
								LED1_OFF(RED_LED1);
						  	LED1_ON(GREEN_LED1);
							
							break;
							case GREEN_LED_t:
								LED1_OFF(GREEN_LED1);
							  LED1_ON(RED_LED1);
							
							break;
							
						}	
				mission_success_send(LED1_OFF_CMD);		
				break;		
						
						
				case	RESET_SCREEN:
				    	reser_screen();
				      usart_state=0;
				      mission_success_send(RESET_SCREEN);	
				break;
					 
			}
}

