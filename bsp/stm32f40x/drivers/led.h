/*
 * FILE                : led.h
 * DESCRIPTION         : This file is for led.c
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
#ifndef __led_h__
#define __led_h__

//--------------------------- Define ---------------------------//
//ºìµÆ<----->PE0

//#define TEST_LED
#ifdef TEST_LED
#define LED0_OFF GPIO_SetBits(GPIOE,GPIO_Pin_0)
#define LED0_ON  GPIO_ResetBits(GPIOE,GPIO_Pin_0)
#define LED1_OFF GPIO_SetBits(GPIOE,GPIO_Pin_1)
#define LED1_ON  GPIO_ResetBits(GPIOE,GPIO_Pin_1)

#else

#define LED0_OFF 
#define LED0_ON  
#define LED1_OFF 
#define LED1_ON  

#endif


int led_hw_initialize(void);

#endif //__led_h__
