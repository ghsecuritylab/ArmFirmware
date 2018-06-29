
/* 接口配置描述 */
/* A0    --- PF0
   D0-D1 --- PD14-PD15
   D2-D3 --- PD0-PD1
   D4-D7 --- PE7-PE10
   CS    --- PG9(NE2) BANK1的NE1~NE4
   RD    --- PD4(NOE)
   WR    --- PD5(NWE)
   INT   --- PA3
   
*/

#include "para.h"

void init_para(void)
{
	//开时钟
	RCC ->AHB1ENR |= (0xF<<3);                               //开启端口使能 D E F G
	RCC ->AHB1ENR |= (1<<0);                                 //使能 PortA
	RCC ->AHB3ENR |= 1<<0;                                   //使能FMC
	//配置IO与复用映射
	GPIO_Set(GPIOF,1<<0 ,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //AO
	GPIO_Set(GPIOD,(3<<14)+(3<<0) ,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //D0-D3
	GPIO_Set(GPIOE,(0xF<<7) ,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //D4-D7
	GPIO_Set(GPIOD,(3<<4) ,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //RD WR
	GPIO_Set(GPIOG,(1<<9) ,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //CS
	GPIO_Set(GPIOA,1<<3 ,GPIO_MODE_IN,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);  //INT
	GPIO_AF_Set(GPIOF,0,12);
	GPIO_AF_Set(GPIOD,14,12);
	GPIO_AF_Set(GPIOD,15,12);
	GPIO_AF_Set(GPIOD,0,12);
	GPIO_AF_Set(GPIOD,1,12);
	GPIO_AF_Set(GPIOE,7,12);
	GPIO_AF_Set(GPIOE,8,12);
	GPIO_AF_Set(GPIOE,9,12);
	GPIO_AF_Set(GPIOE,10,12);	
	GPIO_AF_Set(GPIOD,4,12);
	GPIO_AF_Set(GPIOD,5,12);
	GPIO_AF_Set(GPIOG,9,12);
	//配置FMC
	FMC_Bank1->BTCR[2] = 0x00000000;
	FMC_Bank1->BTCR[3] = 0x00000000;
	FMC_Bank1E->BWTR[2] = 0x00000000;
	FMC_Bank1->BTCR[2]|=1<<14;		//读写使用不同的时序
	FMC_Bank1->BTCR[2]|=1<<12;		//存储器写使能
	
	FMC_Bank1->BTCR[3]|=(7<<8);    //读 数据建立时间（35ns）
	FMC_Bank1->BTCR[3]|=(1<<0);    //读 地址建立时间（5ns）
	
	FMC_Bank1E->BWTR[2]|=6<<8;   	//写数据保存时间(DATAST)	
	FMC_Bank1E->BWTR[2]|=1<<0;		//写地址建立时间(ADDSET)

	FMC_Bank1->BTCR[2]|=1<<0;		//使能BANK1，区域2	  
}

void CH37x_WR_CMD_PORT( unsigned char cmd ) { 				 /* 向CH375的命令端口写入命令,周期不小于2uS,如果单片机较快则延时 */
	u8 temp=60;
	CH37x_CMD_PORT=cmd;
	while(temp--)
		__nop();
}
void CH37x_WR_DAT_PORT( unsigned char dat ) { 				 /* 向CH375的数据端口写入数据,周期不小于1uS,如果单片机较快则延时 */
	u8 temp=4;   //SD卡读，temp=4；U盘读temp=15
	CH37x_DAT_PORT=dat;
	while(temp--)
		__nop();
}
unsigned char CH37x_RD_DAT_PORT(void) {  				     /* 从CH375的数据端口读出数据,周期不小于1uS,如果单片机较快则延时 */
	u8 temp=3;   //SD卡读，temp=3；U盘读temp=15
	while(temp--)
		__nop();		
	return( CH37x_DAT_PORT );
}
unsigned char xReadCH37xStatus() {  				         /* 从CH375的数据端口读出数据,周期不小于1uS,如果单片机较快则延时 */
	u8 temp=15;  
	while(temp--)
		__nop();										  
	return( CH37x_CMD_PORT );
}
	
void xEndCH37xCmd(void)
{
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
	init_para( );  										 /* 接口硬件初始化 */
	delay_ms(50);
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
//	CH37x_WR_DAT_PORT( 0x06 );									     /* 操作USB存储设备 */

	/* 等待模式设置完毕,对于操作SD卡大概需要10mS左右时间,对于操作USB设备大概需要35mS左右时间 */	
	/* 对于并口模式可以多次读取返回数据来判断 */
	for( i = 0; i < 10; i++ )
	{
		delay_ms( 10 );											 
		res = CH37x_RD_DAT_PORT( );
		if( res == CMD_RET_SUCCESS ) 
		{
			xEndCH37xCmd( );  									 /* 并口方式不需要 */
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
	/* 如果连接了CH378的中断引脚则直接查询中断引脚 */
	/* 如果未连接CH378的中断引脚则查询状态端口 */
#ifdef	CH37x_INT_WIRE

	if( CH37x_INT_WIRE == 0x00 )
	{

		return( TRUE );
	
	}
	return( FALSE );  

#else
	return( xReadCH37xStatus( ) & PARA_STATE_INTB ? FALSE : TRUE );  
#endif
}
