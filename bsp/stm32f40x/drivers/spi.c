#include "spi.h"

void rt_hw_spi2_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
	
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource1, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource2, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource3, GPIO_AF_SPI2);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
	//CS(NSS)
	/*
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	*/

	
	SPI_InitStruct.SPI_Direction			= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode 				= SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize 			= SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL 				= SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA 				= SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS 					= SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler 	= SPI_BaudRatePrescaler_4;
	SPI_InitStruct.SPI_FirstBit 			= SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial 		= 7;

	SPI_Init(SPI2, &SPI_InitStruct); 
 
	SPI_Cmd(SPI2, ENABLE);
}

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
rt_uint8_t SPI2_ReadWriteByte(rt_uint8_t TxData)
{		
//	rt_uint8_t retry=0;			
	while((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET)
//	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
//		retry++;
//		if(retry>200)return 0;
	}			  
//	SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
	SPI2->DR = TxData;
//	retry=0;

	while((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET)
//	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
//		retry++;
//		if(retry>200)return 0;
	}	  						    
//	return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据		
	return (SPI2->DR);
}
