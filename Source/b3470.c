/****************************************Copyright (c)****************************************************
**                                	�����˹������Ƽ����޹�˾
**                                    6500 �Լ�ϵͳ                    
**																			@��УԴ
**
**--------------File Info---------------------------------------------------------------------------------
** File Name:               b3470.c
** Last modified Date:      2018-05-18
** Last Version:            v1.1
** Description:             ��������NTC �¶ȴ����� B3470
** 
**--------------------------------------------------------------------------------------------------------
** Created By:              ��УԴ
** Created date:            2018-05-16
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             ��УԴ
** Modified date:           2018-05-18
** Version:                 v1.1
** Description:             ���Ӹ��¶ȶ�ȡ(С������δУ׼)
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
/*********************************************************************************************************
	ͷ�ļ�
*********************************************************************************************************/
#include "b3470.h"
#include "stmflash.h"
/*********************************************************************************************************
	ȫ�ֱ���
*********************************************************************************************************/
u16 table_B3470[B3470_TEMP_RANGE] = {
    705,  738,  771,  806,  841,   // -20  -  -16
    878,  915,   954, 994, 1035,   // -15  -  -11
    1077, 1119, 1163, 1208, 1254,   // -10  -  -6
    1301, 1349, 1398, 1448, 1499,   //  -5  -  -1
    1550, 1603, 1656 ,1710, 1765,  	//  0   -   4
	1820, 1876, 1933, 1990, 2048, 	//  5   -   9
	2106, 2165, 2224, 2283, 2343,   //  10  -   14
	2403, 2463, 2524, 2584, 2645, 	//  15  -   19
	2705, 2766, 2826, 2886, 2946,	//  20  -  24
	3006, 3066, 3125, 3184, 3243,	//  25  -  29
	3301, 3358, 3416, 3472, 3528, 	//  30  -  34
	3584, 3639, 3693, 3747, 3799, 	//  35  -  39
	3852, 3903, 3954, 4004, 4053, 	//  40  -  44
	4101, 4149, 4196, 4242, 4287, 4332   //  45  -  50
};

/*********************************************************************************************************
** Function name:       b3470_init
** Descriptions:        ��ʼ��b3470��Ӧ��ADC GPIO 
** input parameters:    0
** output parameters:   0
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-16
*********************************************************************************************************/
void  b3470_init(void)
{    
  GPIO_InitTypeDef  		GPIO_InitStructure;               //����ṹ�����
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	
  RCC_AHB1PeriphClockCmd(B3470_IO_CLOCK_1, ENABLE);			//ʹ��GPIOAʱ��
	RCC_AHB1PeriphClockCmd(B3470_IO_CLOCK_2, ENABLE);			//ʹ��GPIOAʱ��
  RCC_APB2PeriphClockCmd(B3470_ADC_CLOCK, ENABLE);		 //ʹ��ADC1ʱ��

  //�ȳ�ʼ��ADC1ͨ��5 IO��
  GPIO_InitStructure.GPIO_Pin = B3470_IO_GPIO_PIN_1;//PA5 ͨ��5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;			//ģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//����������   
  GPIO_Init(B3470_IO_GPIO_PORT_1, &GPIO_InitStructure);//��ʼ��  
	
	GPIO_InitStructure.GPIO_Pin = B3470_IO_GPIO_PIN_2;//PA5 ͨ��5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;			//ģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//����������   
  GPIO_Init(B3470_IO_GPIO_PORT_2, &GPIO_InitStructure);//��ʼ��  
 
	RCC_APB2PeriphResetCmd(B3470_ADC_CLOCK,ENABLE);	  //ADC1��λ
	RCC_APB2PeriphResetCmd(B3470_ADC_CLOCK,DISABLE);	//��λ����	 
	
	
 
	
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//����ģʽ
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;//���������׶�֮����ӳ�5��ʱ��
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMAʧ��
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;//Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 
  ADC_CommonInit(&ADC_CommonInitStructure);//��ʼ��
	
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12λģʽ
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;//��ɨ��ģʽ	
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//�ر�����ת��
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//��ֹ������⣬ʹ���������
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//�Ҷ���	
  ADC_InitStructure.ADC_NbrOfConversion = 1;//1��ת���ڹ��������� Ҳ����ֻת����������1 
  ADC_Init(B3470_ADC_PERIPH, &ADC_InitStructure);//ADC��ʼ��
	
	ADC_Cmd(B3470_ADC_PERIPH, ENABLE);//����ADת����	
}				  

/*********************************************************************************************************
** Function name:       bubble_sort_better
** Descriptions:        ð������
** input parameters:    ����ָ�� �� ����
** output parameters:   ���ݵ�����ָ��
** Returned value:      0
** Created by:          ��УԴ
** Created Date:        2018-05-16
*********************************************************************************************************/
static void bubble_sort_better( u16 a[],u16 n)
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


/*********************************************************************************************************
** Function name:       adc_value_get
** Descriptions:        ��ȡB3470_ADC_PERIPH ��Ӧchannel��ת�����
** input parameters:    ADC_Channel
** output parameters:   0
** Returned value:      ADC_Channel_value
** Created by:          ��УԴ
** Created Date:        2018-05-16
*********************************************************************************************************/
u16 adc_value_get(u8 ch)   
{
	  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(B3470_ADC_PERIPH, ch, 1, ADC_SampleTime_480Cycles );	//ADC1,ADCͨ��,480������,��߲���ʱ�������߾�ȷ��			    
  
	ADC_SoftwareStartConv(B3470_ADC_PERIPH);		//ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(B3470_ADC_PERIPH, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(B3470_ADC_PERIPH);	//�������һ��ADC1�������ת�����
}


/*********************************************************************************************************
** Function name:       adc_value_filter
** Descriptions:        ���ADC ת�� ȥ������Сֵ ����ƽ���˲�
** input parameters:    ADC_Channel ת������
** output parameters:   0
** Returned value:      ADC_Channel_filter_value
** Created by:          ��УԴ
** Created Date:        2018-05-16
*********************************************************************************************************/
u16 adc_value_filter(u8 ch)
{
	u32 temp_val=0;
	u16 temp[B3470_ADC_FUN_CONV_TIMES];
	u8 t;
	for(t=0;t<B3470_ADC_FUN_CONV_TIMES;t++)
	{
		temp[t]=adc_value_get(ch);
	}
	bubble_sort_better(temp,B3470_ADC_FUN_CONV_TIMES);
	
	for(t=B3470_ADC_FUN_OFF_TAIL;t<B3470_ADC_FUN_CONV_TIMES-B3470_ADC_FUN_OFF_TAIL;t++)
		temp_val+=temp[t];
	
	return temp_val/(B3470_ADC_FUN_CONV_TIMES-2*B3470_ADC_FUN_OFF_TAIL);
}

/*********************************************************************************************************
** Function name:       adc_value_conv_temperature
** Descriptions:        ���ݴ��ݳ����Ĳ������ұ��
** input parameters:    ADC ����ֵ
** output parameters:   ��
** Returned value:      �¶�
** Created by:          ��УԴ
** Created Date:        2018-05-14
*********************************************************************************************************/
int16_t adc_value_conv_temperature(u16 adc_value)
{
	int16_t temp_h;
	u16 temp_low;
	for(u8 i=0;i<B3470_TEMP_RANGE; i++)
	{
		if(adc_value < table_B3470[i])
		{
			temp_h= (i - 22)*10;
			//TODO ���¶�С������У׼
			temp_low = (adc_value -table_B3470[i-1])*10/(table_B3470[i] -table_B3470[i-1]);
			return temp_h + temp_low;
		}
	}
	return 500;
}

/*********************************************************************************************************
** Function name:       get_temperature
** Descriptions:        ��ѯ��Ӧ�ӿڵ��¶ȴ����� 
** input parameters:    ADC ͨ�� ��B3470_C3 ��B3470_C2��
** output parameters:   ��
** Returned value:      �¶�
** Created by:          ��УԴ
** Created Date:        2018-05-14
*********************************************************************************************************/

int16_t b3470_get_temperature(u8 ch)
{
	return adc_value_conv_temperature(adc_value_filter(ch));
}

/*********************************************************************************************************
** Function name:       b3470_get_temperature_offset
** Descriptions:        ����ƫ��֮����¶����� 
** input parameters:    ADC ͨ�� ��B3470_C3 ��B3470_C2��
** output parameters:   ��
** Returned value:      �¶�
** Created by:          ��УԴ
** Created Date:        2018-05-14
*********************************************************************************************************/

int16_t b3470_get_temperature_offset(u8 ch)
{
	if(ch!=B3470_C3 && ch!=B3470_C2 )
		PRINTF("B3470 channel is not right %d",ch);
	PRINTF("B3470 offset %d",flash_get_para(ch));
	return adc_value_conv_temperature(adc_value_filter(ch))+flash_get_para(ch);
}

/*********************************************************************************************************
  END FILE 
*********************************************************************************************************/
