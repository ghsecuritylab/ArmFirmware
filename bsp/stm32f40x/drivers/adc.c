/*
 * FILE                : adc.c
 * DESCRIPTION         : --
 * Author              : ysloveivy.
 * Copyright           :
 *
 * History
 * --------------------
 * Rev                 : 0.00
 * Date                : 11/21/2015
 *
 * create.
 * --------------------
 */
//------------------------ Include files ------------------------//
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <rtthread.h>

#include "adc.h"
#include "cmd_process.h"
#include "math.h"
#include "usart4.h"
//#include "tc_comm.h"
//--------------------- Function Prototype ----------------------//
static int initialize(void);
static int read_adc(int);
extern rt_event_t evt_record;
extern rt_event_t evt_readhistory;

extern struct rt_messagequeue mq_record;

//-------------------------- Variable ---------------------------//
SYS_ADC_T adc = {
	.initialize = initialize,
	.read = read_adc
};

int jkb_init()
{
	GPIO_InitTypeDef   GPIO_uInitStructure;

	//LED IO初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	GPIO_uInitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;    
	GPIO_uInitStructure.GPIO_Mode = GPIO_Mode_OUT;                          
	GPIO_uInitStructure.GPIO_OType = GPIO_OType_PP;                         
	GPIO_uInitStructure.GPIO_PuPd = GPIO_PuPd_UP;                           
	GPIO_uInitStructure.GPIO_Speed = GPIO_Speed_100MHz;                     

	GPIO_Init(GPIOC,&GPIO_uInitStructure);

	GPIO_ResetBits(GPIOC,GPIO_Pin_0 | GPIO_Pin_1);
}
int jkb_PowerCtrl(int channel,int ctrl)
{
	uint16_t pinnum;
	switch(channel)
	{
		case 0:
			pinnum=GPIO_Pin_0;
		break;
		case 1:
			pinnum=GPIO_Pin_1;
		break;
		default:
			pinnum=GPIO_Pin_0;
		break;
	}
	switch(ctrl)
	{
		case 0:
			GPIO_ResetBits(GPIOC,pinnum);
			break;
		case 1:
			GPIO_SetBits(GPIOC,pinnum);
			break;
		default:
			GPIO_ResetBits(GPIOC,pinnum);
		break;
	}
	
}
int msgqueue_init()
{
	
	
}
void th_readADFunc()
{
	jkb_init();
	jkb_PowerCtrl(0,1);
	msgqueue_init();
	adc.initialize();
	int result;
	char buf[4];
	char tempbuf[4];
	rt_uint32_t event = 0;
	evt_record=rt_event_create("record_screen_btn",RT_IPC_FLAG_FIFO);
	float temp=0.0;
	double counter=0;
	while(1)
	{
		//need event flag
//		if(rt_event_recv(evt_record,
//									btn_START,
//									RT_EVENT_FLAG_AND,
//									RT_WAITING_FOREVER,
//									&event)==RT_EOK)
//		{
		if(1)
		{
			//temp=8*sin(counter)+12;
			
			adc.read(0);
			temp=adc.value[0];
			sprintf(buf,"%f",temp);
			//float mAval_float;
			
		//	memcpy(tempbuf,buf,4);
	//	sscanf(tempbuf,"%f",&mAval_float);
		
			result=rt_mq_send(&mq_record,&buf[0],4);
			if(result==-RT_EFULL)
			{
				rt_thread_delay(50);
			}
			rt_thread_delay(50);
			counter++;
		}
		rt_thread_delay(100);
	}
}
static int initialize(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_CommonInitTypeDef  ADC_CommonInitStructure;
	ADC_InitTypeDef  ADC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);                          
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE); 
	

	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;                               
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                       
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;    
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;     
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;     
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	//???ADC
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                        
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                                
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;          
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv=DISABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                       
	ADC_InitStructure.ADC_NbrOfConversion = 1;                         
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Init(ADC2,&ADC_InitStructure);
	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2,ENABLE);

				
	return 0;
}

void sort(unsigned short int a[], int n)
{
	int i, j, t;
	
	//元素从小到大排列
	for (i = 0; i < n - 1; i++) {
		for (j = 0; j < n - i - 1; j++) {
			if (a[j] > a[j + 1]) {
				t = a[j];
				a[j] = a[j + 1];
				a[j + 1] = t;
			}
		}
	}
}
/*
 * Name                  : read_adc
 * Description           : ---
 * Author                : ysloveivy.
 *
 * History
 * ----------------------
 * Rev                   : 0.00
 * Date                  : 11/21/2015
 *
 * create.
 * ----------------------
 */
#define channels 2
static int read_adc(int channel)
{
	int i,k;
	const int org_100=10;
	const int org_20=2;
	unsigned long int temp[org_20] = {0};
	unsigned long int value;
	unsigned short int data[org_100];
	unsigned char channel_remap[channels] = {ADC_Channel_10,ADC_Channel_11};

	//取得到的100个转换值的中间20个的平均值作为结果
	//连续取20次这样的平均值，再求平均值作为最终结果
	for(k = 0;k < org_20;k++){
		for(i = 0;i < org_100;i++){
if(channel==0)
			{
				ADC_Cmd(ADC1, ENABLE);
				ADC_RegularChannelConfig(ADC1, channel_remap[channel], 1, ADC_SampleTime_480Cycles); //ADC1???????14,15,????,?????480???

				ADC_SoftwareStartConv(ADC1);                                                         //?????ADC1?????????

				while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)== RESET);                                       //??????
			//rt_thread_delay(1);
				data[i] = ADC_GetConversionValue(ADC1);	                                             //?????
			}
else
{
		ADC_Cmd(ADC2, ENABLE);
	ADC_RegularChannelConfig(ADC2, channel_remap[channel], 1, ADC_SampleTime_480Cycles); //ADC1???????14,15,????,?????480???

				ADC_SoftwareStartConv(ADC2);                                                         //?????ADC1?????????

				while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC));                                       //??????
//rt_thread_delay(1);
				data[i] = ADC_GetConversionValue(ADC2);	                                             //?????
}
		}

		sort(data,org_100);

		for(i = 4;i < 6;i++){
			temp[k] += data[i];
		}

		temp[k] = temp[k] / org_20;
	}

	value = 0;
	for(k = 0;k < org_20;k++){
		value += temp[k]; 
	}
	value /= org_20;

	adc.value[channel] = value * ADC_REF / 4096; 

	return value;
}

