/*
 * FILE                : led.c
 * DESCRIPTION         : This file is led driver.
 * Author              : ysloveivy
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
//--------------------------- Include ---------------------------//
#include "led.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

//--------------------------- Function --------------------------//
/*
 * Name                : initialize
 * Description         : ---
 * Author              : ysloveivy.
 *
 * History
 * --------------------
 * Rev                 : 0.00
 * Date                : 11/21/2015
 * 
 * create.
 * --------------------
 */
int led_hw_initialize(void)
{
	GPIO_InitTypeDef   GPIO_uInitStructure;

	//LED IO初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	GPIO_uInitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;    //设置连接三色LED灯的IO端口
	GPIO_uInitStructure.GPIO_Mode = GPIO_Mode_OUT;                          //设置端口为输出模式
	GPIO_uInitStructure.GPIO_OType = GPIO_OType_PP;                         //推挽输出
	GPIO_uInitStructure.GPIO_PuPd = GPIO_PuPd_UP;                           //上拉
	GPIO_uInitStructure.GPIO_Speed = GPIO_Speed_100MHz;                     //设置速度为第三级

	GPIO_Init(GPIOE,&GPIO_uInitStructure);

	//PI5、PI6、PI7接三色LED灯，PI5、PI6、PI7置高电位，灯熄灭
	GPIO_SetBits(GPIOE,GPIO_Pin_0);
	GPIO_SetBits(GPIOE,GPIO_Pin_1);
	return 0;
}
