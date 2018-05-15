#ifndef __ODC_H
#define __ODC_H	 
#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_usart_dma.h"
#include "BUZZER.h"
#include "can.h"
#include <stdio.h>

#define ODC_IDLE		 			0x00	
#define ODC_ENABLE 				0x01
#define ODC_DATAREAD 			0x02
#define ODC_TIMEOUT				0x03
#define ODC_PENDDING			0x04
#define ODC_SUCCESS 			0x05
#define ODC_FAILED				0x06

#define ODC_MAX_RETRY			0x02    //ɨ�����Դ���
#define ODC_WAITTIME 			200
#define ODC_OUTTIME				1000
extern char USART_Buffer2[200];
void one_dimension_code_init(void);
void one_dimension_code_start(void);
uint16_t one_dimension_code_read(void);
void one_dimension_code_mission_polling(void);
	
typedef struct {
  unsigned char running_state;  //����״̬
  u32  timeout;         				//��ʱʱ��
  u32  waittime;								//��ƽת��ʱ��
	u8   length;
	char buf[200];
	char timeoutCount;
}odc_t;

extern odc_t odc;    //�ⲿ����

#endif
