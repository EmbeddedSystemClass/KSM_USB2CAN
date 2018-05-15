/**
  ******************************************************************************
  * @file    led.h
  * $Author: wdluo $
  * $Revision: 229 $
  * $Date:: 2014-05-13 13:00:02 +0800 #$
  * @brief   ��Ϣ����.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, EmbedNet</center>
  *<center><a href="http:\\www.embed-net.com">http://www.embed-net.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#ifndef __BASE_DRIVER_H
#define __BASE_DRIVER_H

#define LED_R   GPIO_Pin_15
#define LED_G   GPIO_Pin_14
#define LED_B   GPIO_Pin_8
/* ѡ��BANK1-NORSRAM1 ���� TFT����ַ��ΧΪ0X60000000~0X63FFFFFF
 * FSMC_A16 ��LCD��DC(�Ĵ���/����ѡ��)��
 * �Ĵ�������ַ = 0X60000000
 * RAM����ַ = 0X60020000 = 0X60000000+2^16*2 = 0X60000000 + 0X20000 = 0X60020000
 * ������Ҫ���ڿ���RS����д�Ĵ���д���ݲ�������Ϊ���ӵ�Һ��RS����A16�ܽţ�����Ҫʹ��ַ��A16Ϊ�ߵ�ƽ����д��ַ��2��16�η�����������ΪҺ����16bit��
 *�����ٳ���2���ó��ĵ�ַ����BANK���ڵĵ�ַ���ɡ���RS��A0������RAM����ַΪbank���ڵ�ַ����2��0�η���2��������8bit������Ҫ����2
 */
#define FSMC_READ_ADDR  0x60020000
#define FSMC_RAM   *(vu16*)((u32)FSMC_READ_ADDR)  //disp Data ADDR

void LED_Config(void);
void LED_On(uint16_t led);
void LED_Off(uint16_t led);
void POWER_Config(uint16_t Power);
void FSMC_Config(void);
void FSMC_DMA_Configuration(uint8_t *pDST_Buffer,uint8_t *pSRC_Buffer,uint32_t BufferSize);

#endif
