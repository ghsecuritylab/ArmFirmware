/*
 * FILE               : exti.c
 * DESCRIPTION        : This file is iCore3 exit driver.
 * Author             : ysloveivy
 * Copyright          :
 *
 * History
 * -------------------
 * Rev                : 0.00
 * Date               : 11/21/2015
 *
 * create.
 * -------------------
 */
//--------------------------- Include ---------------------------//


#include "exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"

//#include "../services/Datalink.h"
//#include "usblink.h"
//---------------------------- Define ---------------------------//
//static int initialize(void);

//-------------------------- Variable ---------------------------//
//EXTI_T exti = {
//	.initialize = initialize
//};
//--------------------------- Function --------------------------//
/*
 * Name               : initialize
 * Description        : ---
 * Author             : ysloveivy.
 *
 * History
 * -------------------
 * Rev                : 0.00
 * Date               : 11/21/2015
 * 
 * create.
 * -------------------
 */
int exti_initialize(void)
{
		EXTI_InitTypeDef   EXTI_InitStructure;
		GPIO_InitTypeDef   GPIO_InitStructure;
		NVIC_InitTypeDef   NVIC_InitStructure;

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);                    //使能GPIOB时钟
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);                  //使能SYSCFG时钟
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);          //PB0与EXTI0连接
		EXTI_InitStructure.EXTI_Line = EXTI_Line0;                             //设置连接中断线15
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;                     //设置中断模式为中断
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;                 //下降沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
		
		//设置中断优先级分组

		NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//EXTI0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		return 0;
}

/*
 * Name               : EXTI15_10_IRQHandler
 * Description        : ---
 * Author             : ysloveivy.
 *
 * History
 * -------------------
 * Rev                : 0.00
 * Date               : 
 * 
 * create.
 * -------------------
 */



void EXTI0_IRQHandler(void)
{
		static unsigned int irqCnt=0;
		if(EXTI_GetITStatus(EXTI_Line0) != RESET)
		{
				EXTI_ClearITPendingBit(EXTI_Line0);
			
				
				irqCnt++;
		}
}
