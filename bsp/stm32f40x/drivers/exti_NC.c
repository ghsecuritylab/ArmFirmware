#include <stm32f10x.h>
#include <rtthread.h>
#include "fsmc.h"
#include "exti.h"
#include "ads1299_config.h"

void NVIC_Configuration(void)
{
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
}

//外部中断初始化程序
void rt_hw_exti_init(void)
{
// 	EXTI_InitTypeDef EXTI_InitStructure;
// 	NVIC_InitTypeDef NVIC_InitStructure;

  //	端口初始化
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);//使能PORTG时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;//PG2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
	GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化GPIOG2
	
	//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;//PA15
	//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
	//	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA15
	
//	//GPIOA.15 中断线以及中断初始化配置   上升沿触发
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);

//	EXTI_InitStructure.EXTI_Line = EXTI_Line10;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器


	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;							
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
//	NVIC_Init(&NVIC_InitStructure); 
}

void EXTI2_Enable(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

//  //	端口初始化
//	GPIO_InitTypeDef GPIO_InitStructure;

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);//使能PORTG时钟

//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;//PG2
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
//	GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化GPIOG2

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

  //GPIOG.2 中断线以及中断初始化配置   下降沿触发
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG,GPIO_PinSource2);

	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

 	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;							
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级1， 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); 
}

void EXTI2_Disable(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  //GPIOG.2 中断线以及中断初始化配置   下降沿触发
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG,GPIO_PinSource2);

	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

 	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;							
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;								//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); 
}

//外部中断2服务程序 
void EXTI2_IRQHandler(void)
{
	/* enter interrupt */
   rt_interrupt_enter();
	
	//发送SRAM Available消息
	rt_event_send(&sram_state_event, SRAM_AVALIABLE);
	
	EXTI_ClearITPendingBit(EXTI_Line2); //清除LINE2上的中断标志位  
	
	/* leave interrupt */
  rt_interrupt_leave();
}

////外部中断10~15服务程序
//void EXTI15_10_IRQHandler(void)
//{
//	/* enter interrupt */
//   rt_interrupt_enter();
//	
//	if(EXTI_GetITStatus(EXTI_Line10) == SET)
//	{
//		EXTI_ClearITPendingBit(EXTI_Line10);  //清除LINE10上的中断标志位		
//	}		
//	
//	if(EXTI_GetITStatus(EXTI_Line11) == SET)
//	{
//		EXTI_ClearITPendingBit(EXTI_Line11);  //清除LINE11上的中断标志位 
//	}		
//	
//	if(EXTI_GetITStatus(EXTI_Line12) == SET)
//	{
//		EXTI_ClearITPendingBit(EXTI_Line12);  //清除LINE12上的中断标志位 
//	}		
//	
//	if(EXTI_GetITStatus(EXTI_Line13) == SET)
//	{
//		EXTI_ClearITPendingBit(EXTI_Line13);  //清除LINE13上的中断标志位 
//	}		
//	
//	if(EXTI_GetITStatus(EXTI_Line14) == SET)
//	{
//		EXTI_ClearITPendingBit(EXTI_Line14);  //清除LINE14上的中断标志位 
//	}		
//	
//	if(EXTI_GetITStatus(EXTI_Line15) == SET)
//	{
//		EXTI_ClearITPendingBit(EXTI_Line15);  //清除LINE15上的中断标志位
//	}	
//	
//	/* leave interrupt */
//  rt_interrupt_leave();
//}
