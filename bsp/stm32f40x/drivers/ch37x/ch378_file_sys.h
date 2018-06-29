/********************************** (C) COPYRIGHT *******************************
* File Name          : FILE_SYS.H
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
#include "para.h"

#ifndef	__CH378_FS_H__
#define __CH378_FS_H__


#define	STRUCT_OFFSET( s, m )      ( (UINT8)( & ((s *)0) -> m ) )				/* 定义获取结构成员相对偏移地址的宏 */

#define	MAX_BYTE_PER_OPERATE	   20480						 				/* 最底层操作每次读写最大字节数 */			

#ifdef	EN_LONG_NAME
#ifndef	LONG_NAME_BUF_LEN
#define	LONG_NAME_BUF_LEN	       ( LONG_NAME_PER_DIR * 20 )			 	    /* 自行定义的长文件名缓冲区长度,最小值为LONG_NAME_PER_DIR*1 */
#endif
#endif


#define EN_SECTOR_ACCESS 1

#define EN_DISK_QUERY  1


/*******************************************************************************/
/* 变量声明 */
extern UINT16 SectorSize;										 				/* 当前U盘每个扇区大小 */  

/*******************************************************************************/
/* 函数外扩 */
extern UINT32 CH378Read32bitDat( void );						 				/* 从CH378芯片读取32位的数据并结束命令 */
extern UINT8 CH378ReadVar8( UINT8 var );						 				/* 读CH378芯片内部的8位变量 */ 
extern void CH378WriteVar8( UINT8 var, UINT8 dat );				 				/* 写CH378芯片内部的8位变量 */
extern UINT32 CH378ReadVar32( UINT8 var );						 				/* 读CH378芯片内部的32位变量 */
extern void CH378WriteVar32( UINT8 var, UINT32 dat );			 				/* 写CH378芯片内部的32位变量 */
extern UINT32 CH378GetTrueLen( void );							 				/* 快速返回上一个命令执行完毕后请求长度所对应的实际长度 */
extern void CH378SetFileName( PUINT8 PathName ); 				 				/* 设置将要操作的文件的文件名、路径名 */
extern UINT8 CH378GetDiskStatus( void );						 				/* 获取磁盘和文件系统的工作状态 */
extern UINT8 CH378GetIntStatus( void );							 				/* 获取中断状态并取消中断请求 */



#ifndef	NO_DEFAULT_CH378_INT
extern UINT8 Wait378Interrupt( void );							 				/* 等待CH378中断(INT#低电平)，返回中断状态码, 超时则返回ERR_USB_UNKNOWN */
#endif

extern UINT8 CH378SendCmdWaitInt( UINT8 mCmd );					 				/* 发出命令码后,等待中断 */
extern UINT8 CH378SendCmdDatWaitInt( UINT8 mCmd, UINT8 mDat );	 				/* 发出命令码和一字节数据后,等待中断 */
extern UINT32 CH378GetFileSize( void );							 				/* 读取当前文件长度 */
extern void CH378SetFileSize( UINT32 filesize );				 				/* 设置当前文件长度 */
extern UINT8 CH378DiskConnect( void );							 				/* 检查U盘/SD卡是否连接 */
extern UINT8 CH378DiskReady( void );							 				/* 初始化磁盘并测试磁盘是否就绪 */
extern UINT16 CH378ReadReqBlock( PUINT8 buf );					 				/* 从当前主机端点的接收缓冲区读取请求数据块 */
extern UINT16 CH378ReadBlock( PUINT8 buf, UINT16 len );			 				/* 从当前主机端点的接收缓冲区读取数据块 */
extern UINT16 CH378ReadOfsBlock( PUINT8 buf, UINT16 offset, UINT16 len ); 		/* 读取内部指定缓冲区指定偏移地址数据块(内部指针自动增加) */
extern UINT16 CH378WriteBlock( PUINT8 buf, UINT16 len );	 	 				/* 向内部指定缓冲区当前偏移地址写入数据块(内部指针自动增加) */	
extern UINT16 CH378WriteOfsBlock( PUINT8 buf, UINT16 offset, UINT16 len );		/* 向内部指定缓冲区指定偏移地址写入数据块(内部指针自动增加) */ 
extern UINT8 CH378FileOpen( PUINT8 PathName );					 				/* 打开文件或者目录(文件夹) */
extern UINT8 CH378FileCreate( PUINT8 PathName );				 				/* 新建文件并打开,如果文件已经存在那么先删除后再新建 */ 
extern UINT8 CH378FileErase( PUINT8 PathName ); 				 				/* 删除文件并关闭 */
extern UINT8 CH378FileClose( UINT8 UpdateSz );					 				/* 关闭当前已经打开的文件或者目录(文件夹) */
extern UINT8 CH378ByteLocate( UINT32 offset );					 				/* 以字节为单位移动当前文件指针 */
extern UINT8 CH378ByteRead( PUINT8 buf, UINT16 ReqCount, PUINT16 RealCount );   /* 以字节为单位从当前位置读取数据块 */
extern UINT8 CH378ByteReadPrepare( UINT16 ReqCount, PUINT16 RealCount );  	    /* 以字节为单位从当前位置读取数据块准备 */
extern UINT8 CH378ByteWrite( PUINT8 buf, UINT16 ReqCount, PUINT16 RealCount );  /* 以字节为单位向当前位置写入数据块 */
extern UINT8 CH378ByteWriteExecute( UINT16 ReqCount, PUINT16 RealCount );	    /* 以字节为单位向当前位置写入数据块 */
extern UINT8 CH378FileQuery( PUINT8 buf );						 				/* CH378查询当前文件的信息(长度、日期、时间、属性) */
extern UINT8 CH378FileModify( UINT32 filesize, UINT16 filedate, UINT16 filetime, UINT8 fileattr ); /* CH378修改当前文件的信息(长度、日期、时间、属性) */
extern UINT8 CH378DirInfoRead( UINT8 index );							 		/* 读取当前文件的目录信息 */	
extern UINT8 CH378DirInfoSave( UINT8 index );							 		/* 保存文件的目录信息 */

#ifdef	EN_SECTOR_ACCESS
extern UINT8 CH378SecLocate( UINT32 offset );  						 			/* 以扇区为单位移动当前文件指针 */
extern UINT8 CH378SecRead( PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount );      /* 以扇区为单位从当前位置读取数据块 */
extern UINT8 CH378SecWrite( PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount );     /* 以扇区为单位在当前位置写入数据块 */ 
extern UINT8 CH378DiskReadSec( PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount ); /* 从U盘读取多个扇区的数据块到缓冲区 */
extern UINT8 CH378DiskWriteSec( PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount );/* 将缓冲区中的多个扇区的数据块写入U盘 */
#endif
	
#ifdef	EN_DISK_QUERY
extern UINT8 CH378DiskCapacity( PUINT32 DiskCap );  			 				/* 查询磁盘物理容量,扇区数 */
extern UINT8 CH378DiskQuery( PUINT32 DiskFre, PUINT32 DiskCap);  				 				/* 查询磁盘剩余空间信息,扇区数 */
#endif
	 						 
#ifdef	EN_DIR_CREATE
extern UINT8 CH378DirCreate( PUINT8 PathName );  				 				/* 新建多级目录下的目录(文件夹)并打开,支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
#endif

#ifdef	EN_LONG_NAME
extern UINT8 CH378GetLongName( PUINT8 PathName, PUINT8 LongName );    			/* 由短文件名或者目录(文件夹)名获得相应的长文件名 */
extern UINT8 CH378GetShortName( PUINT8 PathName, PUINT8 LongName );				/* 由长文件名获取对应的短文件名 */
extern UINT8 CH378CreateLongName( PUINT8 PathName, PUINT8 LongName ); 			/* 新建具有长文件名的文件 */
extern UINT8 CH378CreateLongDir( PUINT8 PathName, PUINT8 LongName );  			/* 新建具有长文件名的目录 */
#endif


#ifdef	EN_OTHER_FUNCTION
extern UINT8 CH378GetICVer( void );												/* 获取CH378芯片版本号 */ 
extern void CH378HardwareReset( void );											/* 执行CH378硬件复位 */	
extern UINT8 CH378CheckExist( void );											/* 检查CH378连接以及通讯是否正常 */
extern UINT8 CH378ClearStall( UINT8 endp );										/* 清除端点错误 */
extern UINT8 CH378AutoEnumDevice( void ); 										/* CH378自动枚举设备 */	
extern UINT8 CH378AutoInitDisk( void );											/* CH378自动初始化USB存储器 */	
extern UINT8 CH378AutoResetDisk( void );										/* CH378复位USB存储器 */	
extern UINT8 CH378GetDiskSize( PUINT8 buf, PUINT8 len );						/* CH378获取USB存储器物理容量 */
extern UINT8 CH378GetDiskInquiry( PUINT8 buf, PUINT8 len );						/* CH378获取USB存储器特性 */
extern UINT8 CH378GetDiskReady( void );											/* CH378检查USB存储器是否准备好 */
extern UINT8 CH378GetDiskReqSense( PUINT8 buf, PUINT8 len );					/* CH378检查USB存储器错误 */ 	
extern UINT8 CH378BlockOnlyCMD( PUINT8 buf, PUINT8 len );						/* CH378执行BulkOnly传输协议的命令 */
#endif

UINT8 CH378EraseAll( void );
void CH378HardwareReset_MINE( void ) ;
void CH378EnterHalfSleep_MINE( void );
void CH378EnterFullSleep_MINE( void ) ;
#endif
