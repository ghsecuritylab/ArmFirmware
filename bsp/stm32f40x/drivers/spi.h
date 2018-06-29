#ifndef __SPI_H__
#define __SPI_H__

//#include <stm32f4xx.h>
#include "stm32f4xx.h"
#include <rtthread.h>


#define SPI2_NSS(n)	(n ? (GPIO_SetBits(GPIOB, GPIO_Pin_12)) : (GPIO_ResetBits(GPIOB, GPIO_Pin_12)))

void rt_hw_spi2_init(void);
rt_uint8_t SPI2_ReadWriteByte(rt_uint8_t TxData);

#endif // STM32_SPI_H_INCLUDED
