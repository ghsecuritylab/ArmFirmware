
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

void gl823k_power_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure PC13 as output */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//enable gpio a clk
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //?? 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   //??
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //??
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //??  
	GPIO_Init(GPIOC, &GPIO_InitStructure);  //??GPIO_InitStructure??????????
	GPIO_SetBits(GPIOC,GPIO_Pin_13);//拉高关闭ch378 power
	
	for(int i=0;i<10000000;i++);// a long delay is needed here
	
	/* Configure PH11 as output */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);//enable gpio a clk
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //?? 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   //??
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //??
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //??  
	GPIO_Init(GPIOI, &GPIO_InitStructure);  //??GPIO_InitStructure??????????
	GPIO_ResetBits(GPIOI,GPIO_Pin_10);//拉低打开GL823 power
}
void gl823k_power_on(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_13);//拉高关闭ch378 power
	for(long i=0;i<1000000;i++);
	GPIO_ResetBits(GPIOI,GPIO_Pin_10);//拉低打开GL823 power
}
void gl823k_power_off(void)
{
	GPIO_SetBits(GPIOI,GPIO_Pin_10);//拉高关闭gl823 power
}
