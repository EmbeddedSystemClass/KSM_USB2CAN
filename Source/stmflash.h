/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               stmflash.h
** Last modified Date:      2018-05-19
** Last Version:            v1.1
** Description:             ��������NTC �¶ȴ����� B3470
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2017-05-16
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             ��УԴ
** Modified date:           2018-05-18
** Version:                 v1.1
** Description:             �����¶�У׼����
**
*********************************************************************************************************/
#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "sys.h"   
#include "b3470.h"
/*********************************************************************************************************
 FLASH ���������Լ�����
 FLASH ƫ������Ϊint16_t  ռ�������ֽ�
 ��д��Ͷ�ȡ������ ����32λΪ����
 ���Թ涨��flash ������д��Ͷ�ȡ�ĳ���
*********************************************************************************************************/
#define FLASH_PARA_NUM			 			 20
#define FLASH_PARA_LEN_32					 (FLASH_PARA_NUM/2)
#define FLASH_PARA_LEN_8					 (FLASH_PARA_NUM*2)
#define FLASH_TEMP1_OFFSET_POS		 11
#define FLASH_TEMP2_OFFSET_POS		 12
#define FLASH_C3_ZL_HIGH		 			 13
#define FLASH_C3_ZL_LOW	 			 		 14
#define FLASH_C2_ZL_HIGH		 			 15
#define FLASH_C2_ZL_LOW	 			 		 16
#define FLASH_TEMP_STATUS			 		 17
#define FLASH_PWOER_STATUS			 	 18

/*********************************************************************************************************
 Ӳ����غ궨��
*********************************************************************************************************/
//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000
//FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
// FLASH һ��1M ����������128K�� 0x0002 0000 = 128*1024*�洢��Ԫ
#define FLASH_SAVE_ADDR  0x08020004 	
									
//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//����5��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//����6��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//����7��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//����8��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//����9��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//����10��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//����11��ʼ��ַ,128 Kbytes  

/*********************************************************************************************************
 FLASH ����״̬���
*********************************************************************************************************/
#define FLASH_IDLE 					0x00
#define	FLASH_WRITE					0x01
#define FLASH_READ					0x02
#define	FLASH_WRITE_SUCCEE	0x03
#define FLASH_READ_SUCCEE		0x04
#define FLASH_FAILE					0x05
#define MEMORY_WIRTE_SUCCEE	0x06
/*********************************************************************************************************
  �ⲿ�����Լ���������
*********************************************************************************************************/
u32 STMFLASH_ReadWord(u32 faddr);		  	//������  
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
void flash_init(void);
void flash_mission_polling(void);
void stmflash_read_reverse(u8 *pBuffer,u32 NumToRead);
void offset_calc(void);
void position_init_to_flash(void);
int16_t flash_get_para(uint8_t device);
/*********************************************************************************************************
  ȫ�ֱ������� �Լ��ⲿ����
*********************************************************************************************************/

typedef struct {
  u8 		current_mission;  			//mission type
	u8		mission_state;
  u8		newdata;         	
	u8 		command_index;
  int16_t	buf[FLASH_PARA_NUM];								//��ƽת��ʱ��
}flash_mission_t;

extern flash_mission_t fmt;

#endif

















