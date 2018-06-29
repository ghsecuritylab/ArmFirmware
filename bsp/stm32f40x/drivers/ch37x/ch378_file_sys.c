/********************************** (C) COPYRIGHT *******************************
* File Name          : FILE_SYS.C
* Author             : MJX
* Version            : V1.20
* Date               : 2015/11/25
* Description        : CH378芯片 文件系统层 V1.2
*                      提供文件系统常用子程序,提供命令打包
*******************************************************************************/



/*******************************************************************************/
/* 简单说明: 
   (1)、不使用的子程序可以注释掉,从而节约单片机的程序ROM空间和数据RAM空间;
   (2)、这里的子程序是通过括号中的变量传递参数,如果参数较多,为了节约RAM,
        也可以参考CH375子程序库改成通过同一全局变量/联合结构CH378_CMD_DATA传递;
   (3)、name 参数是指短文件名, 可以包括根目录符, 但不含有路径分隔符, 总长度不超过1+8+1+3+1字节;
   (4)、PathName 参数是指全路径的短文件名, 包括根目录符、多级子目录及路径分隔符、文件名/目录名;	
   (5)、LongName 参数是指长文件名, 以UNICODE小端顺序编码, 以两个0字节结束, 使用长文件名子程序 */ 

/*******************************************************************************/
/* 部分宏定义说明: 
   (1)、定义 NO_DEFAULT_CH378_INT 用于禁止默认的Wait378Interrupt子程序,禁止后,应用程序必须
        自行定义一个同名子程序;
   (2)、定义 DEF_INT_TIMEOUT 用于设置默认的Wait378Interrupt子程序中的等待中断的超时时间/循环
        计数值, 0则不检查超时而一直等待;
   (3)、定义 EN_DISK_QUERY 用于提供磁盘容量查询和剩余空间查询的子程序,默认是不提供;
   (4)、定义 EN_DIR_CREATE 用于提供新建多级子目录的子程序,默认是不提供;
   (5)、定义 EN_SECTOR_ACCESS 用于提供以扇区为单位读写文件的子程序,默认是不提供;
   (6)、定义 EN_LONG_NAME 用于提供支持长文件名的子程序,默认是不提供;
   (7)、定义 EN_OTHER_FUNCTION 用于提供其它非常用子程序,默认是不提供; */

/*******************************************************************************/
/* 头文件包含 */
#include "ch378_file_sys.h"
#include "delay.h"
#include <string.h>


/*******************************************************************************/
/* 常、变量定义 */
UINT16 SectorSize = 512;										 /* 当前U盘每个扇区大小(默认为512字节) */

/*******************************************************************************/
/* 以下为常用命令打包函数 */
/*******************************************************************************/

/*******************************************************************************
* Function Name  : CH378Read32bitDat
* Description    : 从CH378芯片读取32位的数据并结束命令
* Input          : None
* Output         : None
* Return         : 返回32位数据
*******************************************************************************/
UINT32 CH378Read32bitDat( void )
{
	UINT8  c0, c1, c2, c3;

	c0 = CH37x_RD_DAT_PORT( );
	c1 = CH37x_RD_DAT_PORT( );
	c2 = CH37x_RD_DAT_PORT( );
	c3 = CH37x_RD_DAT_PORT( );
	xEndCH37xCmd( );
	return( c0 | (UINT16)c1 << 8 | (UINT32)c2 << 16 | (UINT32)c3 << 24 );
}

/*******************************************************************************
* Function Name  : CH378ReadVar8
* Description    : 读CH378芯片内部的8位变量
* Input          : addr---8位变量地址
* Output         : None
* Return         : 返回8位数据
*******************************************************************************/
UINT8 CH378ReadVar8( UINT8 addr ) 
{
	UINT8 dat;

	CH37x_WR_CMD_PORT( CMD11_READ_VAR8 );
	CH37x_WR_DAT_PORT( addr );
	dat = CH37x_RD_DAT_PORT( );
	xEndCH37xCmd( );
	return( dat );
}

/*******************************************************************************
* Function Name  : CH378WriteVar8
* Description    : 写CH378芯片内部的8位变量
* Input          : addr---8位变量地址
*                  dat----8位变量值 
* Output         : None
* Return         : None
*******************************************************************************/
void CH378WriteVar8( UINT8 addr, UINT8 dat ) 
{
	CH37x_WR_CMD_PORT( CMD20_WRITE_VAR8 );
	CH37x_WR_DAT_PORT( addr );
	CH37x_WR_DAT_PORT( dat );
	xEndCH37xCmd( );
}

/*******************************************************************************
* Function Name  : CH378ReadVar32
* Description    : 读CH378芯片内部的32位变量
* Input          : addr---32位变量地址
* Output         : None
* Return         : 返回32位数据
*******************************************************************************/
UINT32 CH378ReadVar32( UINT8 addr )
{
	CH37x_WR_CMD_PORT( CMD14_READ_VAR32 );
	CH37x_WR_DAT_PORT( addr );
	return( CH378Read32bitDat( ) );  							 /* 从CH378芯片读取32位的数据并结束命令 */
}

/*******************************************************************************
* Function Name  : CH378WriteVar32
* Description    : 写CH378芯片内部的32位变量
* Input          : addr---32位变量地址
*                  dat---32位变量值
* Output         : None
* Return         : None
*******************************************************************************/
void CH378WriteVar32( UINT8 addr, UINT32 dat ) 
{
	CH37x_WR_CMD_PORT( CMD50_WRITE_VAR32 );
	CH37x_WR_DAT_PORT( addr );
	CH37x_WR_DAT_PORT( (UINT8)dat );
	CH37x_WR_DAT_PORT( (UINT8)( (UINT16)dat >> 8 ) );
	CH37x_WR_DAT_PORT( (UINT8)( dat >> 16 ) );
	CH37x_WR_DAT_PORT( (UINT8)( dat >> 24 ) );
	xEndCH37xCmd( );
}

/*******************************************************************************
* Function Name  : CH378GetTrueLen
* Description    : 快速返回上一个命令执行完毕后请求长度所对应的实际长度
* Input          : None
* Output         : None
* Return         : 返回32位数据
*******************************************************************************/
UINT32 CH378GetTrueLen( void )
{
	UINT8  c0, c1, c2, c3;
	
	CH37x_WR_CMD_PORT( CMD02_GET_REAL_LEN );

	/* 从CH378芯片读取32位的数据并结束命令 */  
	c0 = CH37x_RD_DAT_PORT( );
	c1 = CH37x_RD_DAT_PORT( );
	c2 = CH37x_RD_DAT_PORT( );
	c3 = CH37x_RD_DAT_PORT( );
	xEndCH37xCmd( );
	return( c0 | (UINT16)c1 << 8 | (UINT32)c2 << 16 | (UINT32)c3 << 24 );	
}

/*******************************************************************************
* Function Name  : CH378SetFileName
* Description    : 设置将要操作的文件的完整文件名(路径名)
* Input          : PathName---要设置的文件名或路径名
* Output         : None
* Return         : None
*******************************************************************************/
void CH378SetFileName( PUINT8 PathName ) 
{
	UINT8  i, c;

	if( PathName == NULL )										 /* 如果指针为空,不去从新设置文件名 */
	{
		return;
	}	
	CH37x_WR_CMD_PORT( CMD10_SET_FILE_NAME );
	for( i = MAX_FILE_NAME_LEN; i != 0; --i ) 
	{
		c = *PathName;
		CH37x_WR_DAT_PORT( c );
		if( c == 0 ) 
		{
			break;
		}
		PathName ++;
	}
	xEndCH37xCmd( );
}

/*******************************************************************************
* Function Name  : CH378GetDiskStatus
* Description    : 获取磁盘和文件系统的工作状态
* Input          : None
* Output         : None
* Return         : 返回该值
*******************************************************************************/
UINT8 CH378GetDiskStatus( void )  
{
	return( CH378ReadVar8( VAR8_DISK_STATUS ) );
}

/*******************************************************************************
* Function Name  : CH378GetIntStatus
* Description    : 获取中断状态并取消中断请求
* Input          : None
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378GetIntStatus( void ) 
{
	UINT8  s;

	CH37x_WR_CMD_PORT( CMD01_GET_STATUS );
	s = CH37x_RD_DAT_PORT( );
	xEndCH37xCmd( );
	return( s );
}

/*******************************************************************************
* Function Name  : Wait378Interrupt
* Description    : 等待CH378中断(INT#低电平)
* Input          : None
* Output         : None
* Return         : 返回中断状态码, 超时则返回ERR_USB_UNKNOWN
*******************************************************************************/
#ifndef	NO_DEFAULT_CH378_INT

UINT8 Wait378Interrupt( void ) 
{
#ifdef DEF_INT_TIMEOUT

#if	DEF_INT_TIMEOUT < 1
	while( Query37xInterrupt( ) == FALSE );  					 /* 一直等中断 */
	return( CH378GetIntStatus( ) );  							 /* 检测到中断 */
#else
	UINT32 i;

	for( i = 0; i < DEF_INT_TIMEOUT; i ++ ) 
	{  
		/* 计数防止超时 */
		if( Query37xInterrupt( ) ) 
		{
			return( CH378GetIntStatus( ) );  					 /* 检测到中断 */
		}
		delay_us( 3 );
		/* 在等待CH378中断的过程中,可以做些需要及时处理的其它事情 */
	}
	return( ERR_USB_UNKNOWN );  								 /* 不应该发生的情况 */
#endif

#else
	UINT32 i;

	for( i = 0; i < 5000000; i ++ ) 
	{  
		/* 计数防止超时,默认的超时时间,与单片机主频有关 */
		if ( Query37xInterrupt( ) ) 
		{
			return( CH378GetIntStatus( ) );  					 /* 检测到中断 */
		}
		delay_us( 3 );
		 
		/* 在等待CH378中断的过程中,可以做些需要及时处理的其它事情 */
	}
	return( ERR_USB_UNKNOWN );  								 /* 不应该发生的情况 */
#endif
}
#endif

/*******************************************************************************
* Function Name  : CH378SendCmdWaitInt
* Description    : 发出命令码后,等待中断
* Input          : mCmd---需要执行的命令码
* Output         : None
* Return         : 返回中断状态码
*******************************************************************************/
UINT8 CH378SendCmdWaitInt( UINT8 mCmd )  
{
	CH37x_WR_CMD_PORT( mCmd );
	xEndCH37xCmd( );
	return( Wait378Interrupt( ) );
}

/*******************************************************************************
* Function Name  : CH378SendCmdDatWaitInt
* Description    : 发出命令码和一字节数据后,等待中断
* Input          : mCmd---需要执行的命令码
*                  mDat---1个字节参数
* Output         : None
* Return         : 返回中断状态码
*******************************************************************************/
UINT8 CH378SendCmdDatWaitInt( UINT8 mCmd, UINT8 mDat )
{
	CH37x_WR_CMD_PORT( mCmd );
	CH37x_WR_DAT_PORT( mDat );
	xEndCH37xCmd( );
	return( Wait378Interrupt( ) );
}

/*******************************************************************************
* Function Name  : CH378GetFileSize
* Description    : 读取当前文件长度
* Input          : None
* Output         : None
* Return         : 返回当前文件长度
*******************************************************************************/
UINT32 CH378GetFileSize( void ) 
{
	return( CH378ReadVar32( VAR32_FILE_SIZE ) );
}

/*******************************************************************************
* Function Name  : CH378SetFileSize
* Description    : 设置当前文件长度
*                  注意: 该函数并不是真正设置当前文件长度,而是临时修改当前打开文件
*                        的长度,以便进行某些操作,比如读取文件最后不足一个扇区的数据
* Input          : filesize---文件长度
* Output         : None
* Return         : None
*******************************************************************************/
void CH378SetFileSize( UINT32 filesize ) 
{
	CH378WriteVar32( VAR32_FILE_SIZE, filesize );
}

/*******************************************************************************
* Function Name  : CH378DiskConnect
* Description    : 检查U盘/SD卡是否连接
* Input          : None
* Output         : None
* Return         : 返回中断状态码
*******************************************************************************/
UINT8 CH378DiskConnect( void )  
{
	if( Query37xInterrupt( ) ) 
	{
		CH378GetIntStatus( );  									 /* 检测到中断 */
	}
	return( CH378SendCmdWaitInt( CMD0H_DISK_CONNECT ) );
}

/*******************************************************************************
* Function Name  : CH378DiskReady
* Description    : 初始化磁盘并测试磁盘是否就绪
* Input          : None
* Output         : None
* Return         : 返回中断状态码
*******************************************************************************/
UINT8 CH378DiskReady( void )  
{
	return( CH378SendCmdWaitInt( CMD0H_DISK_MOUNT ) );
}

/*******************************************************************************
* Function Name  : CH378ReadReqBlock
* Description    : 从当前主机端点的接收缓冲区读取请求数据块
* Input          : buf---数据缓冲区	
* Output         : None
* Return         : 返回长度
*******************************************************************************/
UINT16 CH378ReadReqBlock( PUINT8 buf ) 
{
	UINT16 len;
	UINT16 l;
	
	CH37x_WR_CMD_PORT( CMD00_RD_HOST_REQ_DATA );					 /* 写入1个字节命令码 */		
	len = CH37x_RD_DAT_PORT( );
	len += ( ( UINT16 )CH37x_RD_DAT_PORT( ) << 8 );				 /* 读取2个字节数据长度 */
	delay_us( 1 ); 												 /* 对于高速MCU,可在此处适当增加延时 */		
	l = len;
	if( len )
	{
		do
		{		
			*buf = CH37x_RD_DAT_PORT( );	
			buf++;
		}while( --l );		
	}
	xEndCH37xCmd( );
	return( len );
}

/*******************************************************************************
* Function Name  : CH378ReadBlock
* Description    : 从当前主机端点的接收缓冲区读取数据块
* Input          : buf---数据缓冲区
*				   len---读取长度	
* Output         : None
* Return         : 返回长度
*******************************************************************************/
UINT16 CH378ReadBlock( PUINT8 buf, UINT16 len ) 
{
	UINT16 l;
	
	CH37x_WR_CMD_PORT( CMD20_RD_HOST_CUR_DATA );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )len );							 
	CH37x_WR_DAT_PORT( len >> 8 );								 /* 写入2个字节数据长度 */	
	delay_us( 1 ); 												 /* 对于高速MCU,可在此处适当增加延时 */
	if( len )
	{
		l = len;
		do
		{
			*buf = CH37x_RD_DAT_PORT( );
			buf++;
		}while( --l );		
	}
	xEndCH37xCmd( );
	return( len );
}

/*******************************************************************************
* Function Name  : CH378ReadOfsBlock
* Description    : 读取内部指定缓冲区指定偏移地址数据块(内部指针自动增加)
* Input          : buf---数据缓冲区
*				   offset---偏移地址	
*				   len---读取长度	
* Output         : None
* Return         : 返回长度
*******************************************************************************/
UINT16 CH378ReadOfsBlock( PUINT8 buf, UINT16 offset, UINT16 len ) 
{
	UINT16 l;
	
	CH37x_WR_CMD_PORT( CMD40_RD_HOST_OFS_DATA );					 /* 写入1个字节命令码 */
	CH37x_WR_DAT_PORT( ( UINT8 )offset );							 /* 写入2个字节偏移地址 */			
	CH37x_WR_DAT_PORT( offset >> 8 );							
	CH37x_WR_DAT_PORT( ( UINT8 )len );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( len >> 8 );
	delay_us( 1 ); 												 /* 对于高速MCU,可在此处适当增加延时 */							
	if( len )
	{
		l = len;
		do
		{
			*buf = CH37x_RD_DAT_PORT( );
			buf++;
		}while( --l );		
	}
	xEndCH37xCmd( );
	return( len );
}

/*******************************************************************************
* Function Name  : CH378WriteBlock
* Description    : 向内部指定缓冲区当前偏移地址写入数据块(内部指针自动增加)
* Input          : buf---数据缓冲区
*				   len---写入数据长度	
* Output         : None
* Return         : 返回长度
*******************************************************************************/
UINT16 CH378WriteBlock( PUINT8 buf, UINT16 len ) 
{
	UINT16 l;
	
	CH37x_WR_CMD_PORT( CMD20_WR_HOST_CUR_DATA );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )len );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( len >> 8 );	
	delay_us( 1 ); 												 /* 对于高速MCU,可在此处适当增加延时 */						
	if( len )
	{
		l = len;
		do
		{
			CH37x_WR_DAT_PORT( *buf );
			buf++;
		}while( --l );		
	}
	xEndCH37xCmd( );
	return( len );
}

/*******************************************************************************
* Function Name  : CH378WriteOfsBlock
* Description    : 向内部指定缓冲区指定偏移地址写入数据块(内部指针自动增加)
* Input          : buf---数据缓冲区
*				   offset---偏移地址
*				   len---写入数据长度	
* Output         : None
* Return         : 返回长度
*******************************************************************************/
UINT16 CH378WriteOfsBlock( PUINT8 buf, UINT16 offset, UINT16 len ) 
{
	UINT16 l;
	
	CH37x_WR_CMD_PORT( CMD40_WR_HOST_OFS_DATA );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )offset );							 /* 写入2个字节偏移地址 */			
	CH37x_WR_DAT_PORT( offset >> 8 );							
	CH37x_WR_DAT_PORT( ( UINT8 )len );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( len >> 8 );
	delay_us( 1 ); 												 /* 对于高速MCU,可在此处适当增加延时 */							
	if( len )
	{
		l = len;
		do
		{
			CH37x_WR_DAT_PORT( *buf );
			buf++;
		}while( --l );		
	}
	xEndCH37xCmd( );
	return( len );
}

/*******************************************************************************
* Function Name  : CH378FileOpen
* Description    : 打开文件或者目录(文件夹)
* Input          : PathName---要打开的文件或目录的完整路径名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378FileOpen( PUINT8 PathName )  
{
	CH378SetFileName( PathName );  								 /* 设置将要操作的文件的文件名 */
	return( CH378SendCmdWaitInt( CMD0H_FILE_OPEN ) );	
}

/*******************************************************************************
* Function Name  : CH378FileCreate
* Description    : 新建文件并打开,如果文件已经存在那么先删除后再新建
* Input          : PathName---要新建文件的路径及文件名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378FileCreate( PUINT8 PathName ) 
{
	CH378SetFileName( PathName );  								 /* 设置将要操作的文件的文件名 */
	return( CH378SendCmdWaitInt( CMD0H_FILE_CREATE ) );
}

/*******************************************************************************
* Function Name  : CH378FileErase
* Description    : 删除文件并关闭
*                  删除文件,如果已经打开则直接删除,否则对于文件会先打开再删除,支持多级目录路径 
* Input          : PathName---被删除文件的路径名和文件名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378FileErase( PUINT8 PathName ) 
{
	CH378SetFileName( PathName );  
	return( CH378SendCmdWaitInt( CMD0H_FILE_ERASE ) );
}

/*******************************************************************************
* Function Name  : CH378FileClose
* Description    : 关闭当前已经打开的文件或者目录(文件夹)
* Input          : UpdateSz---0---不自动更新文件长度; 1---自动更新文件长度
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378FileClose( UINT8 UpdateSz ) 
{
	return( CH378SendCmdDatWaitInt( CMD1H_FILE_CLOSE, UpdateSz ) );
}

/*******************************************************************************
* Function Name  : CH378ByteLocate
* Description    : 以字节为单位移动当前文件指针
* Input          : offset---偏移字节数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378ByteLocate( UINT32 offset ) 
{
	CH37x_WR_CMD_PORT( CMD4H_BYTE_LOCATE );
	CH37x_WR_DAT_PORT( (UINT8)offset );
	CH37x_WR_DAT_PORT( (UINT8)( (UINT16)offset >> 8 ) );
	CH37x_WR_DAT_PORT( (UINT8)( offset >> 16 ) );
	CH37x_WR_DAT_PORT( (UINT8)( offset >> 24 ) );
	xEndCH37xCmd( );
	return( Wait378Interrupt( ) );
}

/*******************************************************************************
* Function Name  : CH378ByteRead
* Description    : 以字节为单位从当前位置读取数据块
* Input          : buf---缓冲区数据
*				   ReqCount----想要读取的数据长度
*				   RealCount---实际读取的数据长度	
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378ByteRead( PUINT8 buf, UINT16 ReqCount, PUINT16 RealCount ) 
{
	UINT8  s;
	UINT16 len;

	/* 发送字节读取命令 */		
	CH37x_WR_CMD_PORT( CMD2H_BYTE_READ );							 /* 一个字节命令码 */
	CH37x_WR_DAT_PORT( (UINT8)ReqCount );
	CH37x_WR_DAT_PORT( (UINT8)( ReqCount >> 8 ) );				 /* 两个字节数据长度 */
	xEndCH37xCmd( );
	if( RealCount ) 
	{
		*RealCount = 0;
	}
	s = Wait378Interrupt( );
	if( s == ERR_SUCCESS )
	{
		/* 获取实际长度 */
		len = CH378GetTrueLen(  );							 	 /* 当前命令执行后实际返回长度 */
		if( RealCount ) 
		{
			*RealCount = len;
		}
		
		/* 根据实际可读取长度读取数据 */
		if( len )
		{	
			CH378ReadBlock( buf, len );		
		}				
	}
	return( s );	
}

/*******************************************************************************
* Function Name  : CH378ByteReadPrepare
* Description    : 以字节为单位从当前位置读取数据块准备
*                  注意：和CH378ByteRead函数区别：只发送读取命令并执行,但未真正取数据,
*                        该函数执行完毕后,实际数据仍然保存在CH378内部20K缓冲区中
* Input          : ReqCount----想要读取的数据长度
*				   RealCount---实际读取的数据长度	
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378ByteReadPrepare( UINT16 ReqCount, PUINT16 RealCount ) 
{
	UINT8  s;
	UINT16 len;

	/* 发送字节读取命令 */		
	CH37x_WR_CMD_PORT( CMD2H_BYTE_READ );							 /* 一个字节命令码 */
	CH37x_WR_DAT_PORT( (UINT8)ReqCount );
	CH37x_WR_DAT_PORT( (UINT8)( ReqCount >> 8 ) );				 /* 两个字节数据长度 */
	xEndCH37xCmd( );
	if( RealCount ) 
	{
		*RealCount = 0;
	}
	s = Wait378Interrupt( );
	if( s == ERR_SUCCESS )
	{
		/* 获取实际长度 */
		len = CH378GetTrueLen(  );							 	 /* 当前命令执行后实际返回长度 */
		if( RealCount ) 
		{
			*RealCount = len;
		}
	}
	return( s );	
}

/*******************************************************************************
* Function Name  : CH378ByteWrite
* Description    : 以字节为单位向当前位置写入数据块
* Input          : buf---缓冲区数据
*				   ReqCount----想要写入的数据长度
*				   RealCount---实际写入的数据长度	
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378ByteWrite( PUINT8 buf, UINT16 ReqCount, PUINT16 RealCount )  
{
	UINT8  s;

	/* 将要写入的数据预先送到内部缓冲区中 */
	CH378WriteOfsBlock( buf, 0x0000, ReqCount );

	/* 发送字节写入命令 */		
	CH37x_WR_CMD_PORT( CMD2H_BYTE_WRITE );
	CH37x_WR_DAT_PORT( (UINT8)ReqCount );
	CH37x_WR_DAT_PORT( (UINT8)( ReqCount >> 8 ) );
	xEndCH37xCmd( );
	s = Wait378Interrupt( );
	if( s != ERR_SUCCESS )
	{
		if( RealCount ) 
		{
			*RealCount = 0;
		}		
	}
	else
	{
		if( RealCount ) 
		{
			*RealCount = ReqCount;
		}				
	}	
	return( s );
}

/*******************************************************************************
* Function Name  : CH378ByteWriteExecute
* Description    : 以字节为单位向当前位置写入数据块
*                  注意：和CH378ByteWrite函数区别,该函数执行前,数据已经提前存放到CH378内部20K
*                        指定缓冲区。
* Input          : ReqCount----想要写入的数据长度
*				   RealCount---实际写入的数据长度	
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378ByteWriteExecute( UINT16 ReqCount, PUINT16 RealCount )  
{
	UINT8  s;

	/* 发送字节写入命令 */		
	CH37x_WR_CMD_PORT( CMD2H_BYTE_WRITE );
	CH37x_WR_DAT_PORT( (UINT8)ReqCount );
	CH37x_WR_DAT_PORT( (UINT8)( ReqCount >> 8 ) );
	xEndCH37xCmd( );
	s = Wait378Interrupt( );
	if( s != ERR_SUCCESS )
	{
		if( RealCount ) 
		{
			*RealCount = 0;
		}		
	}
	else
	{
		if( RealCount ) 
		{
			*RealCount = ReqCount;
		}				
	}	
	return( s );
}

/*******************************************************************************
* Function Name  : CH378FileQuery
* Description    : CH378查询当前文件的信息(长度、日期、时间、属性)
*                  返回9个字节数据，分别为4个字节长度、2个字节日期、2个字节时间、
*                  个字节属性1 
* Input          : buf---数据缓冲区
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378FileQuery( PUINT8 buf ) 
{
	UINT8  status;
	
	status = CH378SendCmdWaitInt( CMD0H_FILE_QUERY );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}

	CH378ReadReqBlock( buf );
	return( status );
}	

/*******************************************************************************
* Function Name  : CH378FileModify
* Description    : CH378修改当前文件的信息(长度、日期、时间、属性)
* Input          : filesize----新的文件长度,为0xFFFFFFFF则不修改
*				   filedate----新的文件日期,为0xFFFF则不修改
*				   filetime----新的文件时间,为0xFFFF则不修改	
*                  fileattr----新的文件属性,为0xFF则不修改
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378FileModify( UINT32 filesize, UINT16 filedate, UINT16 filetime, UINT8 fileattr ) 
{	
	CH37x_WR_CMD_PORT( CMD40_WR_HOST_OFS_DATA );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( 0x00 );							 		 /* 写入2个字节偏移地址 */			
	CH37x_WR_DAT_PORT( 0x00 );							
	CH37x_WR_DAT_PORT( 0x09 );							 		 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( 0x00 );								
	CH37x_WR_DAT_PORT( (UINT8)filesize );  						 /* 写入4个字节新的文件长度 */					 
	CH37x_WR_DAT_PORT( (UINT8)( filesize >> 8 ) );
	CH37x_WR_DAT_PORT( (UINT8)( filesize >> 16 ) );
	CH37x_WR_DAT_PORT( (UINT8)( filesize >> 24 ) ); 	 		 
	CH37x_WR_DAT_PORT( (UINT8)filedate );  						 /* 写入2个字节新的文件日期 */					 
	CH37x_WR_DAT_PORT( (UINT8)( filedate >> 8 ) );
	CH37x_WR_DAT_PORT( (UINT8)filetime );  						 /* 写入2个字节新的文件时间 */					 
	CH37x_WR_DAT_PORT( (UINT8)( filetime >> 8 ) );
	CH37x_WR_DAT_PORT( (UINT8)fileattr );  						 /* 写入1个字节新的文件属性 */					 
	xEndCH37xCmd( );	
	return( CH378SendCmdWaitInt( CMD0H_FILE_MODIFY ) );
}	

/*******************************************************************************
* Function Name  : CH378DirInfoRead
* Description    : 读取当前打开文件或指定索引处的的目录信息
* Input          : index---目录信息结构在扇区中的索引号
*                  索引号范围为00H--0FH：读取磁盘扇区中对应的当前目录信息结构
*                  索引号为FFH：         读取当前打开的文件所在的目录信息结构
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378DirInfoRead( UINT8 index ) 
{
	return( CH378SendCmdDatWaitInt( CMD1H_DIR_INFO_READ, index ) );
}

/*******************************************************************************
* Function Name  : CH378DirInfoSave
* Description    : 保存当前打开文件或指定索引处的的目录信息
* Input          : index---目录信息结构在扇区中的索引号
*                  索引号范围为00H--0FH：保存磁盘扇区中对应的当前目录信息结构
*                  索引号为FFH：         保存当前打开的文件所在的目录信息结构
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378DirInfoSave( UINT8 index )  
{
	return( CH378SendCmdDatWaitInt( CMD1H_DIR_INFO_SAVE, index ) );
}

/**********************************************************************************************************/
/* 以下为扇区操作相关命令打包函数,可以通过宏EN_SECTOR_ACCESS关闭或开启 */
/**********************************************************************************************************/

#ifdef	EN_SECTOR_ACCESS

/*******************************************************************************
* Function Name  : CH378SecLocate
* Description    : 以扇区为单位移动当前文件指针
* Input          : offset---偏移扇区数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378SecLocate( UINT32 offset )
{
	CH37x_WR_CMD_PORT( CMD4H_SEC_LOCATE );
	CH37x_WR_DAT_PORT( (UINT8)offset );
	CH37x_WR_DAT_PORT( (UINT8)((UINT16)offset >> 8 ) );
	CH37x_WR_DAT_PORT( (UINT8)( offset >> 16 ) );
	CH37x_WR_DAT_PORT( 0 );  										 /* 超出最大文件尺寸 */
	xEndCH37xCmd( );
	return( Wait378Interrupt( ) );
}

/*******************************************************************************
* Function Name  : CH378SecRead
* Description    : 以扇区为单位从当前文件位置读取数据块
* Input          : buf---数据缓冲区
*                  ReqCount------准备读取的扇区数
*                  RealCount-----实际读取的扇区数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378SecRead( PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount ) 
{
	UINT8  s, count;
	UINT8  maxnum;
	UINT16 len, l;

	s = 0;
	if( RealCount ) 
	{
		*RealCount = 0;
	}	
	maxnum = MAX_BYTE_PER_OPERATE / SectorSize; 				 /* 计算当次可以读取的最大扇区数目 */
	
	/* 循环读取扇区数据 */
	while( ReqCount )
	{
		/* 计算单次读取的扇区数 */
		if( ReqCount >= maxnum )
		{
			count = maxnum;
			ReqCount = ReqCount - count;			
		}
		else
		{
			count = ReqCount;
			ReqCount = 0;			
		}

		/* 发送读取扇区命令 */		
		CH37x_WR_CMD_PORT( CMD1H_SEC_READ );
		CH37x_WR_DAT_PORT( count );
		xEndCH37xCmd( );
		s = Wait378Interrupt( );  								 /* 等待中断并获取状态 */
		if( s != ERR_SUCCESS )
		{
			return( s );
		}					

		/* 读取实际可读取的字节数 */
		count = CH378GetTrueLen(  );							 /* 当前命令执行后实际返回长度 */			
		len = count * SectorSize;	

		/* 读取扇区数据到指定缓冲区 */
//		CH378ReadBlock( buf, len );  							 /* 采用下面替换以提高速度 */		
		CH37x_WR_CMD_PORT( CMD20_RD_HOST_CUR_DATA );				 /* 写入1个字节命令码 */		
		CH37x_WR_DAT_PORT( ( UINT8 )len );							 
		CH37x_WR_DAT_PORT( len >> 8 );							 /* 写入2个字节数据长度 */	
		delay_us( 1 ); 											 /* 对于高速MCU,可在此处适当增加延时 */	
		
		/* 可优化以下部分代码以提高读速度 */
		l = len;
		while( l )
		{
			*buf++ = CH37x_RD_DAT_PORT( );			   					
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			*buf++ = CH37x_RD_DAT_PORT( );
			l = l - 16;
		}
		xEndCH37xCmd( );
		buf	= buf + len;						
		if( RealCount ) 
		{
			*RealCount += count;
		}		
		if( count < maxnum )									 /* 读取到文件末尾,提前结束 */		
		{
			return( s );
		}				
	}
	return( s );
}

/*******************************************************************************
* Function Name  : CH378SecWrite
* Description    : 以扇区为单位在当前位置写入数据块
*                  注意: 若写入的扇区数为0,则CH378内部会自动更新文件长度 
* Input          : buf---数据缓冲区
*                  ReqCount------准备写入的扇区数
*                  RealCount-----实际写入的扇区数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378SecWrite( PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount ) 
{
	UINT8  s, count;
	UINT8  maxnum;
	UINT16 len, l;

	if( RealCount ) 
	{
		*RealCount = 0;
	}	
	maxnum = MAX_BYTE_PER_OPERATE / SectorSize; 				 /* 计算当次可以读取的最大扇区数目 */
	
	/* 循环写入扇区数据 */
	do
	{
		/* 计算单次写入的扇区数 */
		if( ReqCount >= maxnum )
		{
			count = maxnum;
			ReqCount = ReqCount - count;			
		}
		else
		{
			count = ReqCount;
			ReqCount = 0;			
		}

		/* 将要写入的数据预先送到内部缓冲区中 */
		len = count * SectorSize;

		/* 可优化此部分代码以提高读速度 */
//		CH378WriteOfsBlock( buf, 0x00, len );						 /* 采用下面的替换以提高速度 */
		CH37x_WR_CMD_PORT( CMD40_WR_HOST_OFS_DATA );					 /* 写入1个字节命令码 */		
		CH37x_WR_DAT_PORT( 0x00 );									 /* 写入2个字节偏移地址 */			
		CH37x_WR_DAT_PORT( 0x00 );							
		CH37x_WR_DAT_PORT( ( UINT8 )len );							 /* 写入2个字节数据长度 */
		CH37x_WR_DAT_PORT( len >> 8 );							
		l = len;
		while( l )
		{
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			CH37x_WR_DAT_PORT( *buf++ );
			l = l - 16;
		}
		xEndCH37xCmd( );
		buf	= buf + len;

		/* 发送写扇区命令 */
		CH37x_WR_CMD_PORT( CMD1H_SEC_WRITE );
		CH37x_WR_DAT_PORT( count );
		xEndCH37xCmd( );
		s = Wait378Interrupt( );  								 /* 等待中断并获取状态 */
		if( s != ERR_SUCCESS )
		{
			return( s );
		}
		if( RealCount ) 
		{
			*RealCount += count;
		}		
	}while( ReqCount );
	return( s );
}

/*******************************************************************************
* Function Name  : CH378DiskReadSec
* Description    : 从U盘读取多个扇区的数据块到缓冲区
* Input          : buf---数据缓冲区
*                  iLbaStart------准备读取的线性起始扇区号
*                  iSectorCount---准备读取的扇区数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378DiskReadSec( PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount ) 
{	
	UINT8  s, count;
	UINT8  maxnum;
	UINT16 len;
	UINT32 seclba;	

	s = 0;
	seclba = iLbaStart;
	maxnum = MAX_BYTE_PER_OPERATE / SectorSize; 				 /* 计算当次可以读取的最大扇区数目 */

	/* 循环读取扇区数据 */
	while( iSectorCount )
	{
		/* 计算单次读取的扇区数 */
		if( iSectorCount >= maxnum )
		{
			count = maxnum;
			iSectorCount = iSectorCount - count;			
		}
		else
		{
			count = iSectorCount;
			iSectorCount = 0;			
		}
	
		/* 发送读取扇区命令 */		
		CH37x_WR_CMD_PORT( CMD5H_DISK_READ );  					 /* 从USB存储器读扇区 */
		CH37x_WR_DAT_PORT( (UINT8)seclba );  						 /* LBA的最低8位 */
		CH37x_WR_DAT_PORT( (UINT8)( (UINT16)seclba >> 8 ) );
		CH37x_WR_DAT_PORT( (UINT8)( seclba >> 16 ) );
		CH37x_WR_DAT_PORT( (UINT8)( seclba >> 24 ) ); 	 		 /* LBA的最高8位 */
		CH37x_WR_DAT_PORT( count );  							 	 /* 扇区数 */
		xEndCH37xCmd( );
		s = Wait378Interrupt( );  								 /* 等待中断并获取状态 */
		if( s != ERR_SUCCESS )
		{
			return( s );
		}					
		
		/* 读取扇区数据到指定缓冲区 */
		len = count * SectorSize;
		CH378ReadBlock( buf, len );
		buf	= buf + len;		
		seclba = seclba + count;	
	}
	return( s );
}

/*******************************************************************************
* Function Name  : CH378DiskWriteSec
* Description    : 将缓冲区中的多个扇区的数据块写入U盘
* Input          : buf---数据缓冲区
*                  iLbaStart------写入的线起始性扇区号
*                  iSectorCount---准备写入的扇区数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378DiskWriteSec( PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount )  
{	
	UINT8  s, count;
	UINT8  maxnum;
	UINT16 len;
	UINT32 seclba;
	
	s = 0;
	seclba = iLbaStart;
	maxnum = MAX_BYTE_PER_OPERATE / SectorSize; 				 /* 计算当次可以读取的最大扇区数目 */

	/* 循环写入扇区数据 */
	while( iSectorCount )
	{
		/* 计算单次写入的扇区数 */
		if( iSectorCount >= maxnum )
		{
			count = maxnum;
			iSectorCount = iSectorCount - count;			
		}
		else
		{
			count = iSectorCount;
			iSectorCount = 0;			
		}	

		/* 将要写入的数据预先送到内部缓冲区中 */
		len = count * SectorSize;
		CH378WriteOfsBlock( buf, 0x00, len );
		buf	= buf + len;
				
		/* 发送写扇区命令 */
		CH37x_WR_CMD_PORT( CMD5H_DISK_WRITE );  					 /* 向USB存储器写扇区 */
		CH37x_WR_DAT_PORT( (UINT8)seclba );  						 /* LBA的最低8位 */
		CH37x_WR_DAT_PORT( (UINT8)( (UINT16)seclba >> 8 ) );
		CH37x_WR_DAT_PORT( (UINT8)( seclba >> 16 ) );
		CH37x_WR_DAT_PORT( (UINT8)( seclba >> 24 ) );  			 /* LBA的最高8位 */
		CH37x_WR_DAT_PORT( count );  								 /* 扇区数 */
		xEndCH37xCmd( );		
		s = Wait378Interrupt( );  								 /* 等待中断并获取状态 */
		if( s != ERR_SUCCESS )
		{
			return( s );
		}
		seclba = seclba + count;		
	}
	return( s );
}

#endif

/**********************************************************************************************************/
/* 以下为磁盘查询操作相关命令打包函数,可以通过宏EN_DISK_QUERY关闭或开启 */
/**********************************************************************************************************/

#ifdef	EN_DISK_QUERY

/*******************************************************************************
* Function Name  : CH378DiskCapacity
* Description    : 查询磁盘物理容量(扇区数)
* Input          : DiskCap---扇区数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378DiskCapacity( PUINT32 DiskCap )  
{
	UINT8  s;
	
	s = CH378SendCmdWaitInt( CMD0H_DISK_CAPACITY );
	if ( s == ERR_SUCCESS ) 
	{  
		CH37x_WR_CMD_PORT( CMD00_RD_HOST_REQ_DATA );				 /* 写入1个字节命令码 */		
		CH37x_RD_DAT_PORT( );							
		CH37x_RD_DAT_PORT( );						 	
		*DiskCap = CH378Read32bitDat( );  						 /* 从CH378芯片读取32位的数据并结束命令 */
	}
	else 
	{
		*DiskCap = 0;
	}
	return( s );
}

/*******************************************************************************
* Function Name  : CH378DiskQuery
* Description    : 查询磁盘剩余空间信息(扇区数)
* Input          : DiskFre---扇区数
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378DiskQuery( PUINT32 DiskFre, PUINT32 DiskCap )  
{
	UINT8  s;
	UINT8  c0, c1, c2, c3;
	UINT8  c4, c5, c6, c7;

	s = CH378SendCmdWaitInt( CMD0H_DISK_QUERY );
	if( s == ERR_SUCCESS ) 
	{  		
		CH37x_WR_CMD_PORT( CMD00_RD_HOST_REQ_DATA );				 /* 写入1个字节命令码 */		
		CH37x_RD_DAT_PORT( );							
		CH37x_RD_DAT_PORT( );						 		 	     /* 读取2个字节数据长度 */		
				
		c4 = CH37x_RD_DAT_PORT( );  									 /* CH378_CMD_DATA.DiskQuery.mTotalSector */
		c5 = CH37x_RD_DAT_PORT( );
		c6 = CH37x_RD_DAT_PORT( );
		c7 = CH37x_RD_DAT_PORT( );

		*DiskCap = c4 | (UINT16)c5 << 8 | (UINT32)c6 << 16 | (UINT32)c7 << 24;
		
		c0 = CH37x_RD_DAT_PORT( );  								 /* CH378_CMD_DATA.DiskQuery.mFreeSector */
		c1 = CH37x_RD_DAT_PORT( );
		c2 = CH37x_RD_DAT_PORT( );
		c3 = CH37x_RD_DAT_PORT( );
		*DiskFre = c0 | (UINT16)c1 << 8 | (UINT32)c2 << 16 | (UINT32)c3 << 24;
		CH37x_RD_DAT_PORT( );  									 /* CH378_CMD_DATA.DiskQuery.mDiskFat */
		xEndCH37xCmd( );
	}
	else 
	{
		*DiskFre = 0;
	}
	return( s );
}

#endif

/**********************************************************************************************************/
/* 以下为目录操作相关命令打包函数,可以通过宏EN_DIR_CREATE关闭或开启 */
/**********************************************************************************************************/

#ifdef	EN_DIR_CREATE

/*******************************************************************************
* Function Name  : CH378DirCreate
* Description    : 在根目录下新建目录(文件夹)并打开,如果目录已经存在那么直接打开
* Input          : PathName---要建立的文件夹的路径及文件夹名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378DirCreate( PUINT8 PathName ) 
{
	CH378SetFileName( PathName );  								 /* 设置将要操作的文件夹的文件夹名 */
	return( CH378SendCmdWaitInt( CMD0H_DIR_CREATE ) );
}
#endif

/**********************************************************************************************************/
/* 以下为长文件名操作相关命令打包函数,可以通过宏EN_LONG_NAME关闭或开启 */
/**********************************************************************************************************/

#ifdef	EN_LONG_NAME

/*******************************************************************************
* Function Name  : CH378GetLongName
* Description    : 由短文件名或者目录(文件夹)名获得相应的长文件名
*                  需要输入短文件名的完整路径PathName
* Input          : PathName---短文件名的完整路径
* Output         : LongName---返回的长文件名,以UNICODE小端编码,以双0结束
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378GetLongName( PUINT8 PathName, PUINT8 LongName )
{
	UINT8  status;	
	
	CH378SetFileName( PathName );  								 /* 设置将要操作的文件的完整路径名 */
	status = CH378SendCmdWaitInt( CMD10_GET_LONG_FILE_NAME );
	if( status == ERR_SUCCESS )
	{
		/* 如果该命令有数据返回,则读取该数据 */
		CH378ReadReqBlock( LongName );
	}
	return( status );		
}

/*******************************************************************************
* Function Name  : CH378GetShortName
* Description    : 由短文件名或者目录(文件夹)部分路径及完整长文件名获得相应的短文件名
* Input          : LongName---完整的长文件名,以UNICODE小端编码,以双0结束
* Output         : PathName---短文件名的完整路径
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378GetShortName( PUINT8 PathName, PUINT8 LongName )
{
	UINT8  status;	
	UINT16	count;

	/* 设置将要操作的短文件的完整路径名 */	
	CH378SetFileName( PathName );  								 /* 设置将要操作的文件的部分路径名 */

	/* 设置长文件名 */
	for( count = 0; count < LONG_NAME_BUF_LEN; count += 2 ) 
	{
		if ( *(PUINT16)( &LongName[ count ] ) == 0 ) 
		{
			break;  											 /* 到结束位置 */
		}
	}
	if( ( count == 0 ) || ( count >= LONG_NAME_BUF_LEN ) || ( count > LONE_NAME_MAX_CHAR ) ) 
	{
		return( ERR_LONG_NAME_ERR );  							 /* 长文件名无效 */
	}	
	count = count + 2;											 /* 长文件名最后两个字节为0x00 */		
	CH37x_WR_CMD_PORT( CMD10_SET_LONG_FILE_NAME );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )count );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( count >> 8 );							
	do
	{
		CH37x_WR_DAT_PORT( *LongName++ );
	}while( --count );		
	xEndCH37xCmd( );

	/* 获取对应的短文件名 */
	status = CH378SendCmdWaitInt( CMD1H_GET_SHORT_FILE_NAME );
	if( status == ERR_SUCCESS )
	{
		/* 如果该命令有数据返回,则读取该数据 */
		CH378ReadReqBlock( PathName );
	}
	return( status );		
}

/*******************************************************************************
* Function Name  : CH378CreateLongName
* Description    : 新建具有长文件名的文件
*                  需要输入短文件名的完整路径PathName(请事先参考FAT规范由长文件名自行产生),
*                  需要输入以UNICODE小端编码的以双0结束的长文件名LongName
* Input          : PathName---短文件名的完整路径
*                  LongName---以UNICODE小端编码的以双0结束的长文件名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378CreateLongName( PUINT8 PathName, PUINT8 LongName )  
{
	UINT16	count;			
	
	/* 设置将要操作的短文件的完整路径名 */
	CH378SetFileName( PathName );  								 
	
	/* 设置长文件名 */
	for( count = 0; count < LONG_NAME_BUF_LEN; count += 2 ) 
	{
		if ( *(PUINT16)( &LongName[ count ] ) == 0 ) 
		{
			break;  											 /* 到结束位置 */
		}
	}
	if( ( count == 0 ) || ( count >= LONG_NAME_BUF_LEN ) || ( count > LONE_NAME_MAX_CHAR ) ) 
	{
		return( ERR_LONG_NAME_ERR );  							 /* 长文件名无效 */
	}	
	count = count + 2;											 /* 长文件名最后两个字节为0x00 */		
	CH37x_WR_CMD_PORT( CMD10_SET_LONG_FILE_NAME );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )count );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( count >> 8 );							
	do
	{
		CH37x_WR_DAT_PORT( *LongName++ );
	}while( --count );		
	xEndCH37xCmd( );	
	
	/* 执行创建长文件名命令 */
	return( CH378SendCmdWaitInt( CMD0H_LONG_FILE_CREATE ) );
}

/*******************************************************************************
* Function Name  : CH378CreateLongDir
* Description    : 新建具有长文件名的目录
* Input          : PathName---短文件名的完整路径
*                  LongName---以UNICODE小端编码的以双0结束的长文件名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378CreateLongDir( PUINT8 PathName, PUINT8 LongName )  
{
	UINT16	count;			

	/* 设置将要操作的短文件的完整路径名 */	
	CH378SetFileName( PathName );  								
	
	/* 设置长文件名 */
	for( count = 0; count < LONG_NAME_BUF_LEN; count += 2 ) 
	{
		if ( *(PUINT16)( &LongName[ count ] ) == 0 ) 
		{
			break;  											 /* 到结束位置 */
		}
	}
	if( ( count == 0 ) || ( count >= LONG_NAME_BUF_LEN ) || ( count > LONE_NAME_MAX_CHAR ) ) 
	{
		return( ERR_LONG_NAME_ERR );  							 /* 长文件名无效 */
	}	
	count = count + 2;											 /* 长文件名最后两个字节为0x00 */		
	CH37x_WR_CMD_PORT( CMD10_SET_LONG_FILE_NAME );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )count );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( count >> 8 );							
	do
	{
		CH37x_WR_DAT_PORT( *LongName++ );
	}while( --count );		
	xEndCH37xCmd( );	
	
	/* 执行创建长目录名命令 */
	return( CH378SendCmdWaitInt( CMD0H_LONG_DIR_CREATE ) );
}

/*******************************************************************************
* Function Name  : CH378OpenLongName
* Description    : 通过长文件名打开文件
* Input          : PathName---短文件所在目录的路径名
*                  LongName---以UNICODE小端编码的以双0结束的长文件名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378OpenLongName( PUINT8 PathName, PUINT8 LongName ) 
{
	UINT8  status;	
	UINT16	count;

	/* 设置将要操作的短文件的路径名 */	
	CH378SetFileName( PathName );  								 /* 设置将要操作的文件的部分路径名 */

	/* 设置长文件名 */
	for( count = 0; count < LONG_NAME_BUF_LEN; count += 2 ) 
	{
		if ( *(PUINT16)( &LongName[ count ] ) == 0 ) 
		{
			break;  											 /* 到结束位置 */
		}
	}
	if( ( count == 0 ) || ( count >= LONG_NAME_BUF_LEN ) || ( count > LONE_NAME_MAX_CHAR ) ) 
	{
		return( ERR_LONG_NAME_ERR );  							 /* 长文件名无效 */
	}	
	count = count + 2;											 /* 长文件名最后两个字节为0x00 */		
	CH37x_WR_CMD_PORT( CMD10_SET_LONG_FILE_NAME );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )count );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( count >> 8 );							
	do
	{
		CH37x_WR_DAT_PORT( *LongName++ );
	}while( --count );		
	xEndCH37xCmd( );

	/* 打开长文件名对应的文件 */
	status = CH378SendCmdWaitInt( CMD0H_LONG_FILE_OPEN );
	return( status );	
}

/*******************************************************************************
* Function Name  : CH378EraseLongName
* Description    : 通过长文件名删除文件
* Input          : PathName---短文件所在目录的路径名
*                  LongName---以UNICODE小端编码的以双0结束的长文件名
* Output         : None
* Return         : 返回中断状态
*******************************************************************************/
UINT8 CH378EraseLongName( PUINT8 PathName, PUINT8 LongName ) 
{
	UINT8  status;	
	UINT16	count;

	/* 设置将要操作的短文件的路径名 */	
	CH378SetFileName( PathName );  								 /* 设置将要操作的文件的部分路径名 */

	/* 设置长文件名 */
	for( count = 0; count < LONG_NAME_BUF_LEN; count += 2 ) 
	{
		if ( *(PUINT16)( &LongName[ count ] ) == 0 ) 
		{
			break;  											 /* 到结束位置 */
		}
	}
	if( ( count == 0 ) || ( count >= LONG_NAME_BUF_LEN ) || ( count > LONE_NAME_MAX_CHAR ) ) 
	{
		return( ERR_LONG_NAME_ERR );  							 /* 长文件名无效 */
	}	
	count = count + 2;											 /* 长文件名最后两个字节为0x00 */		
	CH37x_WR_CMD_PORT( CMD10_SET_LONG_FILE_NAME );					 /* 写入1个字节命令码 */		
	CH37x_WR_DAT_PORT( ( UINT8 )count );							 /* 写入2个字节数据长度 */
	CH37x_WR_DAT_PORT( count >> 8 );							
	do
	{
		CH37x_WR_DAT_PORT( *LongName++ );
	}while( --count );		
	xEndCH37xCmd( );

	/* 删除长文件名对应的文件 */
	status = CH378SendCmdWaitInt( CMD0H_LONG_FILE_ERASE );
	return( status );	
}


#endif

/**********************************************************************************************************/
/* 以下为非常用命令打包函数,可以通过宏EN_OTHER_FUNCTION关闭或开启,也可以选择性的开启 */
/**********************************************************************************************************/

#ifdef	EN_OTHER_FUNCTION

/*******************************************************************************
* Function Name  : CH378GetICVer
* Description    : 获取CH378芯片版本号
* Input          : None
* Output         : None
* Return         : 返回芯片版本号
*******************************************************************************/
UINT8 CH378GetICVer( void ) 
{
	UINT8  s;

	CH37x_WR_CMD_PORT( CMD01_GET_IC_VER );
	s = CH37x_RD_DAT_PORT( );
	xEndCH37xCmd( );
	return( s );
}

/*******************************************************************************
* Function Name  : CH378HardwareReset
* Description    : 执行CH378硬件复位
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH378HardwareReset( void ) 
{
	CH37x_WR_CMD_PORT( CMD00_RESET_ALL );
	xEndCH37xCmd( );
	delay_ms( 50 );												 /* 等待复位完成 */
}

/*******************************************************************************
* Function Name  : CH378EnterHalfSleep
* Description    : 使能CH378进入半睡眠状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH378EnterHalfSleep( void ) 
{
	CH37x_WR_CMD_PORT( CMD10_ENTER_SLEEP ); 
	CH37x_WR_DAT_PORT( HALF_SLEEP_MODE ); 						 /* 半睡眠模式 */	
	xEndCH37xCmd( );
	delay_ms( 50 );												 /* 等待唤醒完成 */
}

/*******************************************************************************
* Function Name  : CH378EnterFullSleep
* Description    : 使能CH378进入深度睡眠状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH378EnterFullSleep( void ) 
{
	CH37x_WR_CMD_PORT( CMD10_ENTER_SLEEP ); 
	CH37x_WR_DAT_PORT( FULL_SLEEP_MODE ); 						 /* 深度睡眠模式 */	
	xEndCH37xCmd( );
	delay_ms( 50 );												 /* 等待唤醒完成 */
}

/*******************************************************************************
* Function Name  : CH378CheckExist
* Description    : 检查CH378连接以及通讯是否正常
* Input          : None
* Output         : None
* Return         : 返回ERR_SUCCESS表示CH378连接以及通讯正常;
*                  返回ERR_USB_UNKNOWN表示不正常,需要检查错误
*******************************************************************************/
UINT8 CH378CheckExist( void ) 
{
	UINT8  dat;

	CH37x_WR_CMD_PORT( CMD11_CHECK_EXIST );  						 /* 测试单片机与CH378之间的通讯接口 */
	CH37x_WR_DAT_PORT( 0x78 );
	dat = CH37x_RD_DAT_PORT( );
	xEndCH37xCmd( );
	if( dat != 0x87 ) 
	{
		/* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */
		return( ERR_USB_UNKNOWN );  
	}
	else
	{
		return( ERR_SUCCESS );
	}	
}

/*******************************************************************************
* Function Name  : CH378ClearStall
* Description    : 清除端点错误
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378ClearStall( UINT8 endp ) 
{
	return( CH378SendCmdDatWaitInt( CMD1H_CLR_STALL, endp ) );	 /* 主机方式: 控制传输-清除端点错误 */	
}

/*******************************************************************************
* Function Name  : CH378EnumDevice
* Description    : CH378手动枚举USB设备
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378SelfEnumDevice( PUINT8 buf ) 
{
	UINT8  status;
	
	/* 获取设备描述符 */
	status = CH378SendCmdDatWaitInt( CMD1H_GET_DESCR, 0x01 );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	status = CH378ReadReqBlock( buf );
	
	/* 可进行描述符分析 */

	/* 设置地址 */
	status = CH378SendCmdDatWaitInt( CMD1H_SET_ADDRESS, 0x05 );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	
	/* 获取配置描述符 */
	status = CH378SendCmdDatWaitInt( CMD1H_GET_DESCR, 0x02 );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	status = CH378ReadReqBlock( buf );

	/* 可进行描述符分析 */
	
	/* 设置配置值 */
	status = CH378SendCmdDatWaitInt( CMD1H_SET_CONFIG, buf[ 5 ] );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	return( status );		
}

/*******************************************************************************
* Function Name  : CH378AutoEnumDevice
* Description    : CH378自动枚举设备
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378AutoEnumDevice( void ) 
{ 
	return( CH378SendCmdWaitInt( CMD0H_AUTO_SETUP ) );		
}

/*******************************************************************************
* Function Name  : CH378AutoInitDisk
* Description    : CH378自动初始化USB存储器
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378AutoInitDisk( void ) 
{
	return( CH378SendCmdWaitInt( CMD0H_DISK_INIT ) );		
}

/*******************************************************************************
* Function Name  : CH378AutoResetDisk
* Description    : CH378复位USB存储器
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378AutoResetDisk( void ) 
{
	return( CH378SendCmdWaitInt( CMD0H_DISK_RESET ) );		
}

/*******************************************************************************
* Function Name  : CH378GetDiskSize
* Description    : CH378获取USB存储器物理容量
* Input          : None
* Output         : buf---返回数据包
*				   len---返回数据长度(一般为8个字节)
* Return         : None
*******************************************************************************/
UINT8 CH378GetDiskSize( PUINT8 buf, PUINT8 len ) 
{
	UINT8  status;
	
	status = CH378SendCmdWaitInt( CMD0H_DISK_SIZE );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	*len = CH378ReadReqBlock( buf );
	return( status );
}	

/*******************************************************************************
* Function Name  : CH378GetDiskInquiry
* Description    : CH378获取USB存储器特性
* Input          : None
* Output         : buf---返回数据包
*				   len---返回数据长度(一般为36个字节)
* Return         : None
*******************************************************************************/
UINT8 CH378GetDiskInquiry( PUINT8 buf, PUINT8 len ) 
{
	UINT8  status;
	
	status = CH378SendCmdWaitInt( CMD0H_DISK_INQUIRY );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	*len = CH378ReadReqBlock( buf );
	return( status );	
}	

/*******************************************************************************
* Function Name  : CH378GetDiskReady
* Description    : CH378检查USB存储器是否准备好
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 CH378GetDiskReady( void ) 
{
	UINT8  status;
	
	status = CH378SendCmdWaitInt( CMD0H_DISK_READY );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	return( status );	
}	

/*******************************************************************************
* Function Name  : CH378GetDiskReqSense
* Description    : CH378检查USB存储器错误
* Input          : None
* Output         : buf---返回数据包
*				   len---返回数据长度
* Return         : None
*******************************************************************************/
UINT8 CH378GetDiskReqSense( PUINT8 buf, PUINT8 len ) 
{
	UINT8  status;
	
	status = CH378SendCmdWaitInt( CMD0H_DISK_R_SENSE );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}
	*len = CH378ReadReqBlock( buf );
	return( status );
}	

/*******************************************************************************
* Function Name  : CH378BlockOnlyCMD
* Description    : CH378执行BulkOnly传输协议的命令
* Input          : buf---需要执行的BulkOnly命令包; 
* Output         : buf---执行BulkOnly命令返回数据包
*                  len---返回数据包长度  
* Return         : None
*******************************************************************************/
UINT8 CH378BlockOnlyCMD( PUINT8 buf, PUINT8 len ) 
{
	UINT8  status;

	/* 先通过CMD40_WR_HOST_OFS_DATA命令写入CBW包 */	
	CH378WriteOfsBlock( buf, 0x00, 31 );
	
	/* 再发送命令码 */
	status = CH378SendCmdWaitInt( CMD0H_DISK_BOC_CMD );
	if( status != ERR_SUCCESS )
	{
		return( status );
	}

	/* 如果该命令有数据返回,则读取该数据 */
	*len = CH378ReadReqBlock( buf );
	return( status );
}		



#endif


/*******************************************************************************
* Function Name  : CH378EraseAll
* Description    : 
* Input          : 
* Output         :  
* Return         : None
*******************************************************************************/
UINT8 CH378EraseAll( void ) 
{
		UINT8  status;
	
		unsigned char  buf[ 64 ];												 /* 数据缓冲区 */
		memset(buf, 0, sizeof(buf));
	
		strcpy( (char*)buf, "\\*" );  							 /* C51子目录不存在则列出根目录下的文件 */

		status = CH378FileOpen( buf );  					 /* 枚举多级目录下的文件或者目录 */
		while( status == USB_INT_DISK_READ ) 
		{  
				/* 枚举到匹配的文件 */
				CH378ReadReqBlock( buf );  						 /* 读取枚举到的文件的完整路径名 */
				
				unsigned char  fileName[ 64 ];
				memset(fileName, 0, sizeof(fileName));
				memcpy( fileName, buf, 8); 
				memcpy( fileName + 8, ".TXT", 4); 
				
				status = CH378FileErase(fileName);  				/* 删除文件 */

				rt_thread_delay(10);
			
				if( status != ERR_SUCCESS ) 
				{
						//return 3;
				}
		
		
			//usart4.printf( "***EnumName: %s\n",buf );				 /* 打印名称,原始8+3格式,未整理成含小数点分隔符 */
			CH37x_WR_CMD_PORT( CMD0H_FILE_ENUM_GO );  			 /* 继续枚举文件和目录 */
			xEndCH37xCmd( );
			status = Wait378Interrupt( );
		}
		//if( status != ERR_MISS_FILE ) 
		//
			//return status;  						 /* 操作出错 */
		//}
		return status;
}		

void CH378HardwareReset_MINE( void ) 
{
	CH37x_WR_CMD_PORT( CMD00_RESET_ALL );
	xEndCH37xCmd( );
	delay_ms( 50 );												 /* 等待复位完成 */
}
/*******************************************************************************
* Function Name  : CH378EnterHalfSleep
* Description    : 使能CH378进入半睡眠状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH378EnterHalfSleep_MINE( void ) 
{
	CH37x_WR_CMD_PORT( CMD10_ENTER_SLEEP ); 
	CH37x_WR_DAT_PORT( HALF_SLEEP_MODE ); 						 /* 半睡眠模式 */	
	xEndCH37xCmd( );
	delay_ms( 50 );												 /* 等待唤醒完成 */
}

/*******************************************************************************
* Function Name  : CH378EnterFullSleep
* Description    : 使能CH378进入深度睡眠状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH378EnterFullSleep_MINE( void ) 
{
	CH37x_WR_CMD_PORT( CMD10_ENTER_SLEEP ); 
	CH37x_WR_DAT_PORT( FULL_SLEEP_MODE ); 						 /* 深度睡眠模式 */	
	xEndCH37xCmd( );
	delay_ms( 50 );												 /* 等待唤醒完成 */
}