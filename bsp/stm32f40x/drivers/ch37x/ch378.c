#include "ch378.h"
#include "stdio.h"
#include "delay.h"
#include "rtthread.h"

void ch378_power_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure PH11 as output */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);//enable gpio a clk
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //?? 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   //??
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //??
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //??  
	GPIO_Init(GPIOI, &GPIO_InitStructure);  //??GPIO_InitStructure??????????
	GPIO_SetBits(GPIOI,GPIO_Pin_10);//拉高关掉GL823 power

	for(long i=0;i<10000000;i++);
		/* Configure PC13 as output */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//enable gpio a clk
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //?? 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   //??
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //??
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //??  
	GPIO_Init(GPIOC, &GPIO_InitStructure);  //??GPIO_InitStructure??????????
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);//拉低打开ch378 power
}
void ch378_power_on(void)
{
	GPIO_SetBits(GPIOI,GPIO_Pin_10);//拉高关闭gl823 power
	for(long i=0;i<10000000;i++);
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);//拉低打开ch378 power
}
void ch378_power_off(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_13);//拉高关闭ch378 power
}
static void init_spi(void)
{
	RCC ->AHB1ENR |= (1<<0);    //使能PA
	RCC ->APB2ENR |= (1<<12);   //使能SPI1
	GPIO_Set(GPIOA,1<<4 ,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //CS
	SPI1_CS = 1;
	GPIO_Set(GPIOA,1<<3 ,GPIO_MODE_IN,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //INT
	GPIO_Set(GPIOA,0x7<<5 ,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);
	GPIO_AF_Set(GPIOA,5,5);
	GPIO_AF_Set(GPIOA,6,5);
	GPIO_AF_Set(GPIOA,7,5);
	//SPI初始化
	SPI1 ->CR1 = 0X0000;
	SPI1 ->CR1 |= (1<<9);   //软件nss管理
	SPI1 ->CR1 |= (1<<8);
	SPI1 ->CR1 |= (1<<3);   //四分频 24M
	SPI1 ->CR1 |= (1<<2);   //主模式
//	SPI1 ->CR1 |= (1<<1);   //CPOL
//	SPI1 ->CR1 |= (1<<0);   //CPHA
	SPI1 ->I2SCFGR &= ~(1<<11); //选择SPI模式
	SPI1 ->CR1 |= (1<<6);   //启动SPI
}
//spi收发一个字节
u8 spi_trans_byte(u8 dat)
{
	while(!(SPI1->SR & 0x02));   //发送为空
	SPI1 ->DR = dat;
	while(!(SPI1->SR & 0x01));   //接收非空
	return (SPI1 ->DR );
}

void CH37x_WR_CMD_PORT( unsigned char cmd ) { 				 /* 向CH375的命令端口写入命令,周期不小于2uS,如果单片机较快则延时 */
	u8 temp=20;                 
	SPI1_CS = 0;
	spi_trans_byte(cmd);
	while(temp--)
		__nop();
}
void CH37x_WR_DAT_PORT( unsigned char dat ) { 				 /* 向CH375的数据端口写入数据,周期不小于1uS,如果单片机较快则延时 */
	u8 temp=1;   //SD卡读，temp=1； U盘字节temp=1，扇区5
	spi_trans_byte(dat);
	while(temp--)
		__nop();
}
unsigned char CH37x_RD_DAT_PORT(void) {  				     /* 从CH375的数据端口读出数据,周期不小于1uS,如果单片机较快则延时 */
	u8 temp=3;   //SD卡读，temp=3； U盘字节temp=0，扇区10
	while(temp--)
		__nop();		
	return( spi_trans_byte(0xff) );
}
void xEndCH37xCmd(void)
{
	SPI1_CS = 1;
}

/*******************************************************************************
* Function Name  : mInitCH378Host
* Description    : 初始化CH378
* Input          : None
* Output         : None
* Return         : 返回操作状态
*******************************************************************************/
u8 mInitCH37xHost( void ) 
{
	u8 i, res;

	/* 检测CH378连接是否正常 */
	ch378_power_init();
	init_spi( );  										 /* 接口硬件初始化 */
	delay_ms(500);

	CH37x_WR_CMD_PORT( CMD11_CHECK_EXIST );  						 /* 测试单片机与CH378之间的通讯接口 */
	CH37x_WR_DAT_PORT( 0x65 );
	res = CH37x_RD_DAT_PORT( );
	xEndCH37xCmd( );  											 /* 并口方式不需要 */
	
	if( res != 0x9A ) 
	{
		/* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */
		return( res );  
	}

	/* 设置CH378工作模式 */
	CH37x_WR_CMD_PORT( CMD11_SET_USB_MODE );  						 /* 设备USB工作模式 */
	CH37x_WR_DAT_PORT( 0x03 );									     /* 操作SD存储设备 */
//	CH37x_WR_DAT_PORT( 0x07 );									     /* 操作USB存储设备 */

	/* 等待模式设置完毕,对于操作SD卡大概需要10mS左右时间,对于操作USB设备大概需要35mS左右时间 */	
	/* 对于并口模式可以多次读取返回数据来判断 */
	for( i = 0; i < 10; i++ )
	{
		delay_ms( 10 );											 
		//rt_thread_delay(1);
		
		res = CH37x_RD_DAT_PORT( );
		if( res == CMD_RET_SUCCESS ) 
		{
			xEndCH37xCmd( );   									 /* 并口方式不需要 */
			return( ERR_SUCCESS );
		}
	}
	xEndCH37xCmd( );  											 /* 并口方式不需要 */
	return( ERR_USB_UNKNOWN );  						  	     /* 设置模式错误 */
}

/*******************************************************************************
* Function Name  : Query378Interrupt
* Description    : 查询CH378中断(INT#低电平)
* Input          : None
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
u8 Query37xInterrupt( void )
{
	if( CH37x_INT_WIRE == 0x00 )
	{
		return( TRUE );
	
	}
	return( FALSE );  
}

