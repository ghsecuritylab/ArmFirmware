#include <stm32f10x.h>
#include <rtthread.h>
#include "adc.h"
#include "dma.h"

void DMA_NVIC_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;							
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x04;	//抢占优先级3， 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能中断通道
	NVIC_Init(&NVIC_InitStructure); 
}

void DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能DMA时钟
	
	/*初始化ADC DMA通道*/
	DMA_DeInit(DMA1_Channel1);//复位DMA通道1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (rt_uint32_t)&(ADC1->DR); //定义DMA通道外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (rt_uint32_t)adc_value; //定义DMA通道存储器地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//指定外设为源地址
	DMA_InitStructure.DMA_BufferSize = ADC_BUFFER_SIZE;//定义DMA缓冲区大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//当前外设地址不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//当前存储器地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//定义外设数据位宽16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //定义存储器数据宽度16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//DMA通道操作模式为环形缓冲模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//DMA通道优先级高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//禁止DMA通道存储器到存储器传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);//初始化DMA通道1
	DMA_Cmd(DMA1_Channel1, ENABLE); //使能DMA通道1
	DMA_NVIC_init();
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);//使能DMA传输完成中断
}

void DMA1_Channel1_IRQHandler(void)
{
	/* enter interrupt */
   rt_interrupt_enter();
	
	if(DMA_GetITStatus(DMA1_IT_TC1) != RESET)
	{
		get_battery_voltage();		
		
		DMA_ClearITPendingBit(DMA1_IT_TC1);
	}
	
	/* leave interrupt */
  rt_interrupt_leave();
}
