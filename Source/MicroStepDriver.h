#ifndef __BSP_ADVANCETIME_H
#define __BSP_ADVANCETIME_H


#include "stm32f4xx.h"


#define  CW  0
#define CCW 1

#define TRUE 1
#define FALSE 0

#define Pulse_width 20

//ϵͳ״̬
struct GLOBAL_FLAGS {
  //�����������������ʱ��ֵΪ1
  unsigned char running:1;
  //�����ڽ��յ�����ʱ��ֵΪ1
  unsigned char cmd:1;
  //���������������ʱ,ֵΪ1
  unsigned char out_ena:1;
};

extern struct GLOBAL_FLAGS status;
extern int stepPosition;

#define T1_FREQ 					100000     //��ʱ��Ƶ��
#define FSPR    					200         //���������Ȧ����
#define SPR     					(FSPR*10)  //10ϸ�ֵĲ���

#define CIRCUL_RATIO			10
#define	CIRCUL_STEP 			19386
#define RESRT_OFFSET			500
#define RESET_STEP      	CIRCUL_STEP+RESRT_OFFSET
#define ANGLE_CALC_STEP		CIRCUL_STEP/360
#define HALF_CIRCUL_STEP 	CIRCUL_STEP/2
#define REVERSER_OFFSET		0
#define ANGLE_PER_CALC		CIRCUL_STEP*360
// ��ѧ������ ����MSD_Move�����ļ򻯼���
#define ALPHA (2*3.14159/SPR)                    // 2*pi/spr
#define A_T_x100 ((long)(ALPHA*T1_FREQ*100))     // (ALPHA
#define T1_FREQ_148 ((int)((T1_FREQ*0.676)/100)) // divided by 100 and scaled by 0.676
#define A_SQ (long)(ALPHA*2*10000000000)         // 
#define A_x20000 (int)(ALPHA*20000)              // ALPHA*20000
    
//�ٶ�����״̬
#define STOP  		0
#define ACCEL 		1
#define DECEL 		2
#define RUN   		3
#define FINISH 		4
#define	WATI_STOP	5
typedef struct {
  //�������״̬
  unsigned char run_state : 4;
  //������з���
  unsigned char dir : 1;
	
  //��һ��������ʱ���ڣ�����ʱΪ���ٶ�����
  unsigned int step_delay;
	
  //��ʼ���ٵ�λ��
  unsigned int decel_start;
  //���پ���
  signed int decel_val;
  //��С��ʱ��������ٶȣ�
  signed int min_delay;
  //���ٻ��߼��ټ�����
  signed int accel_count;
	unsigned int step_all;
	u8 last_dir;
	u16 position;
	u16 position_to_move;
	unsigned char wait_count;
	u16 last_pos;
	u16 last_pos_times;
} speedRampData;


// ��ʹ�ò�ͬ�Ķ�ʱ����ʱ�򣬶�Ӧ��GPIO�ǲ�һ���ģ����Ҫע��
// ��������ʹ�ö�ʱ��TIM2
#define            MSD_PULSE_TIM                    TIM2
#define            MSD_PULSE_TIM_APBxClock_FUN      RCC_APB1PeriphClockCmd
#define            MSD_PULSE_TIM_CLK                RCC_APB1Periph_TIM2

// ��ʱ�����PWMͨ����PA0��ͨ��1
#define            MSD_PULSE_OCx_Init               TIM_OC2Init
#define            MSD_PULSE_OCx_PreloadConfig      TIM_OC2PreloadConfig
// ��ʱ���ж�
#define            MSD_PULSE_TIM_IRQ                TIM2_IRQn
#define            MSD_PULSE_TIM_IRQHandler         TIM2_IRQHandler

// PWM �źŵ�Ƶ�� F = TIM_CLK/{(ARR+1)*(PSC+1)}
#define            MSD_PULSE_TIM_PERIOD             (10-1)
#define            MSD_PULSE_TIM_PSC                (168-1)

// ��������������ͨ��
#define            MSD_PULSE_AHBxClock_FUN          RCC_AHB1PeriphClockCmd
#define            MSD_PULSE_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define            MSD_PULSE_PORT                   GPIOA
#define            MSD_PULSE_PIN                    GPIO_Pin_1
#define            MSD_PULSE_PIN_AF                 GPIO_AF_TIM2
#define            MSD_PULSE_PIN_SOURCE             GPIO_PinSource1

// ��������������
#define            MSD_DIR_AHBxClock_FUN            RCC_AHB1PeriphClockCmd
#define            MSD_DIR_GPIO_CLK                 RCC_AHB1Periph_GPIOC
#define            MSD_DIR_PORT                     GPIOC
#define            MSD_DIR_PIN                      GPIO_Pin_11

// ����������ʹ������
#define            MSD_ENA_AHBxClock_FUN            RCC_AHB1PeriphClockCmd
#define            MSD_ENA_GPIO_CLK                 RCC_AHB1Periph_GPIOC
#define            MSD_ENA_PORT                     GPIOC
#define            MSD_ENA_PIN                      GPIO_Pin_10


#define DIR(a)	if (a == CW)	\
					GPIO_SetBits(MSD_DIR_PORT,MSD_DIR_PIN);\
					else		\
					GPIO_ResetBits(MSD_DIR_PORT,MSD_DIR_PIN)
					
					

/**************************��������********************************/

void MSD_Init(void);
void MSD_ENA(FunctionalState NewState);
void MSD_Move(signed int step, unsigned int accel, unsigned int decel, unsigned int speed);
void record_steps(signed char inc);
u16 get_steps(signed char inc);
void record_position(signed char inc);
extern volatile speedRampData srd;
					uint8_t motor_stop_check(void);
#endif	/* __BSP_ADVANCETIME_H */


