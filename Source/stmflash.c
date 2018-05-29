/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               stmflash.c
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

#include "stmflash.h"
#include "can.h"
#include "MSD_test.h"  
/*********************************************************************************************************
 FLASH ����ȫ�ֱ���
*********************************************************************************************************/
flash_mission_t fmt; //STM32�ڲ�FLASH��д ��������	   
 
/*********************************************************************************************************
** Function name:       STMFLASH_ReadWord
** Descriptions:        ��ȡָ����ַ�İ���(16λ����) 
** input parameters:    faddr:����ַ 
** output parameters:   0
** Returned value:      ��Ӧ����.
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
}  
/*********************************************************************************************************
** Function name:       STMFLASH_GetFlashSector
** Descriptions:        ��ȡĳ����ַ���ڵ�flash����
** input parameters:    flash��ַ
** output parameters:   0
** Returned value:      0~11,��addr���ڵ�����
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
uint16_t STMFLASH_GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}

/*********************************************************************************************************
** Function name:       STMFLASH_Read
** Descriptions:        ��ָ����ַ��ʼд��ָ�����ȵ�����
** �ر�ע��:��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����
**         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
**         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
**         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
** 				 �ú�����OTP����Ҳ��Ч!��������дOTP��!
** 				 OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F
** input parameters:    WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!) 
**											pBuffer:����ָ�� 
**											NumToWrite:��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
  FLASH_Status status = FLASH_COMPLETE;
	u32 addrx=0;
	u32 endaddr=0;	
  if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//�Ƿ���ַ
	FLASH_Unlock();									//���� 
 // FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���
 		
	addrx=WriteAddr;				//д�����ʼ��ַ
	endaddr=WriteAddr+NumToWrite*4;	//д��Ľ�����ַ
	if(addrx<0X1FFF0000)			//ֻ�����洢��,����Ҫִ�в�������!!
	{
		while(addrx<endaddr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{   
				status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V֮��!!
				if(status!=FLASH_COMPLETE)break;	//����������
			}else addrx+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(WriteAddr<endaddr)//д����
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//д������
			{ 
				break;	//д���쳣
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
//  FLASH_DataCacheCmd(ENABLE);	//FLASH��������,�������ݻ���
	FLASH_Lock();//����
} 


/*********************************************************************************************************
** Function name:       STMFLASH_Read
** Descriptions:        ��ָ����ַ��ʼ����ָ�����ȵ�����
** input parameters:    ReadAddr:��ʼ��ַ pBuffer:����ָ�� NumToRead:��(4λ)��
** output parameters:   pBuffer:����ָ��
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)   	
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}


/*********************************************************************************************************
** Function name:       flash_mission_polling
** Descriptions:        flash ��������
** input parameters:    mission.c �н��ܵ�can ��ָ��
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
void flash_mission_polling(){
	static u8 flash_u8_buf[FLASH_PARA_LEN_8];
	static u8	upload[2];
	switch(fmt.mission_state){
	
	case FLASH_IDLE:
		if(fmt.newdata){
			offset_calc();
			fmt.newdata=0;
		}
		break;
	
	case FLASH_WRITE:    //д���ڴ�
		
		STMFLASH_Write(FLASH_SAVE_ADDR,(u32*)fmt.buf,FLASH_PARA_LEN_32);
		fmt.mission_state=FLASH_WRITE_SUCCEE;
	
	break;
	
	case FLASH_READ:   //��ȡ�ڴ�
		stmflash_read_reverse(flash_u8_buf,FLASH_PARA_LEN_32);
		fmt.mission_state=FLASH_READ_SUCCEE;
	break;
	
	case FLASH_WRITE_SUCCEE:  //д��ɹ�
			
	
		mission_success_send(fmt.current_mission);
		fmt.newdata=1;
		fmt.mission_state=FLASH_IDLE;
	
		break;
	
	case FLASH_READ_SUCCEE:
		//MSB AND LSB problem
		if(fmt.command_index*2+1 > FLASH_PARA_LEN_8)
			break;
		upload[0]=flash_u8_buf[fmt.command_index*2];
		upload[1]=flash_u8_buf[fmt.command_index*2+1];
		
		action_value_send(upload,2,fmt.current_mission);
	
		fmt.mission_state=FLASH_IDLE;
	
		break;
	
	case MEMORY_WIRTE_SUCCEE:
		mission_success_send(fmt.current_mission);
		fmt.newdata=1;
		fmt.mission_state=FLASH_IDLE;
		break;
	
	case FLASH_FAILE:
		break;
	}
}

/*********************************************************************************************************
** Function name:       flash_init
** Descriptions:        flash ������ʼ�� ��FLASH_SAVE_ADDR��ȡ���ݵ� ȫ�ֱ���
** input parameters:    0
** output parameters:   fmt.buf
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
void flash_init(){
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32 *)fmt.buf,FLASH_PARA_LEN_32);
	fmt.newdata=1;
}

void stmflash_read_reverse(u8 *pBuffer,u32 NumToRead){
	u8 temp[256];
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32*)temp,NumToRead);
	for(u8 i=0;i<NumToRead*4;i+=2){
		*(pBuffer+i)=temp[i+1];
		*(pBuffer+i+1)=temp[i];
	}
}
/*********************************************************************************************************
** Function name:       offset_calc
** Descriptions:        ƫ��������  ƫ��ֵ = ����ֵ - ʵ��ֵ
**											motor.offset ���������ƫ��ֵ
**											fmt.buf			 FLASHд���ʵ��ֵ
**											angle				 ����ֵ
** input parameters:    0
** output parameters:   fmt.buf
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
void offset_calc(void){			
	motor.offset[1]=fmt.buf[1];
	motor.offset[2]=fmt.buf[2]-angle(1,1,1);
	motor.offset[3]=fmt.buf[3]-angle(1,1,2);
	motor.offset[4]=fmt.buf[4]-angle(1,1,3);
	motor.offset[5]=fmt.buf[5]-angle(2,1,1);
	motor.offset[6]=fmt.buf[6]-angle(2,1,2);
	motor.offset[7]=fmt.buf[7]-angle(2,1,3);
	motor.offset[8]=fmt.buf[8]-angle(3,1,1);
	motor.offset[9]=fmt.buf[9]-angle(3,1,2);
	motor.offset[10]=fmt.buf[10]-angle(3,1,3);
	//PRINTF("fmt.temperature_offset_c2 %d fmt.temperature_offset_c3 %d\r\n",fmt.temperature_offset_c2,fmt.temperature_offset_c3);
}

/*********************************************************************************************************
** Function name:       position_init_to_flash
** Descriptions:        �ѵ��Ҫ�ߵ�������д���ڴ� ������
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2017-05-01
*********************************************************************************************************/
void position_init_to_flash(void)
{
	fmt.buf[1]=0;
	fmt.buf[2]=step_to_move_calc(1,1,1);
	fmt.buf[3]=step_to_move_calc(1,1,2);
	fmt.buf[4]=step_to_move_calc(1,1,3);
	fmt.buf[5]=step_to_move_calc(2,20,1);
	fmt.buf[6]=step_to_move_calc(2,21,2);
	fmt.buf[7]=step_to_move_calc(2,24,3);
	fmt.buf[8]=step_to_move_calc(3,1,1);
	fmt.buf[9]=step_to_move_calc(3,1,2);
	fmt.buf[10]=step_to_move_calc(3,1,3);
	fmt.current_mission=FLASH_MISSION_WRITE;
	fmt.mission_state=FLASH_WRITE;
	fmt.newdata=1;
}

/*********************************************************************************************************
** Function name:       flash_get_para
** Descriptions:        ��ȡ��Ӧ�豸��ƫ�Ʋ���
** input parameters:    B3470_C2 B3470_C3
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-20
*********************************************************************************************************/
int16_t flash_get_para(uint8_t device)
{
	switch(device)
	{
		case B3470_C2:
			return fmt.buf[FLASH_TEMP1_OFFSET_POS];
		case B3470_C3:
			return fmt.buf[FLASH_TEMP2_OFFSET_POS];
		case FLASH_C3_ZL_HIGH:
			
			if(fmt.buf[FLASH_C3_ZL_HIGH] == -1)
				return 75;
			else
				return fmt.buf[FLASH_C3_ZL_HIGH];
			
		case FLASH_C3_ZL_LOW:
			
			if(fmt.buf[FLASH_C3_ZL_LOW] == -1)
				return 45;
			else
				return fmt.buf[FLASH_C3_ZL_LOW];
			
		case FLASH_C2_ZL_HIGH:
			
			if(fmt.buf[FLASH_C2_ZL_HIGH] == -1)
				return 100;
			else
				return fmt.buf[FLASH_C2_ZL_HIGH];		
			
		case FLASH_C2_ZL_LOW:
			
			if(fmt.buf[FLASH_C2_ZL_LOW] == -1)
				return -20;
			else
				return fmt.buf[FLASH_C2_ZL_LOW];		
			
		case FLASH_C2_STOP_TIME:
			
			if(fmt.buf[FLASH_C2_STOP_TIME] == -1)
				return 60;
			else
				return fmt.buf[FLASH_C2_STOP_TIME];	
		
		case FLASH_LM35_ZL_HIGH:
			if(fmt.buf[FLASH_LM35_ZL_HIGH] == -1)
				return 70;
			else
				return fmt.buf[FLASH_LM35_ZL_HIGH];	
			
		case FLASH_LM35_ZL_LOW:
			if(fmt.buf[FLASH_LM35_ZL_LOW] == -1)
				return 45;
			else
				return fmt.buf[FLASH_LM35_ZL_LOW];			

		case FLASH_LM35_ON_TIME:
			if(fmt.buf[FLASH_LM35_ON_TIME] == -1)
				return (30*60);
			else
				return fmt.buf[FLASH_LM35_ON_TIME];	
			
		case FLASH_LM35_OFF_TIME:
			if(fmt.buf[FLASH_LM35_OFF_TIME] == -1)
				return (60);
			else
				return fmt.buf[FLASH_LM35_OFF_TIME];					
		
		case FLASH_LM35_DIS_OFFSET:
			if(fmt.buf[FLASH_LM35_DIS_OFFSET] == -1)
				return -40;
			else
				return fmt.buf[FLASH_LM35_DIS_OFFSET];	
		default:
			PRINTF("fmt got no para of %d",device);
			return 0;
	}
}




