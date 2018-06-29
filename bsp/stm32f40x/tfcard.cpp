#include "tfcard.hpp"
#include "rtthread.h"
#include "ch378.h"
#include "ch378_file_sys.h"
#include "delay.h"
#define TFCARD_MAX_SIZE_PER_WRITE				1024*16
#define TFCARD_BLOCK_SIZE								512

#define TFCARD_SINGLE_FILE_MAX_LEN			1024*1024*10			//单个文件最大长度  单位：字节
rt_bool_t tfcard_ready = RT_FALSE;
/*线程参数*/
#define TFCARD_WRITE_THREAD_STACK_SIZE       	TFCARD_MAX_SIZE_PER_WRITE+1024*2
#define TFCARD_WRITE_THREAD_PRIORITY         	25
#define TFCARD_WRITE_THREAD_TIMESLICE        	50
/*线程ID*/
rt_thread_t tfcard_write_tid = RT_NULL;
rt_err_t tfcard_init(void)
{
		rt_uint8_t s=0;
		tfcard_ready = RT_FALSE;
	
		s = mInitCH37xHost();	
		if(ERR_SUCCESS!=s)
		{
				return RT_ERROR;
		}
		delay_ms( 100 );
		
		/* 检查U盘或者SD卡是否连接,等待其插入 */
		if( CH378DiskConnect( ) != ERR_SUCCESS ) 
				return RT_ERROR;
		
		delay_ms( 200 ); /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
		
		/* 对于检测到USB设备的,最多等待100*50mS,主要针对有些MP3太慢,对于检测到USB设备并且连接DISK_MOUNTED的,
		最多等待5*50mS,主要针对DiskReady不过的 */
		for( int i = 0; i < 100; i ++ ) 
		{  
				/* 最长等待时间,100*50mS */
				delay_ms( 50 );
				//printf( "Ready ?\n" );
				s = CH378DiskReady( );  						 /* 初始化磁盘并测试磁盘是否就绪 */
				if( s == ERR_SUCCESS ) 
				{
						break;  										 /* 准备好 */
				}
				else if( s == ERR_DISK_DISCON ) 
				{
						return RT_ERROR;//goto UnknownUsbDevice; 							 /* 检测到断开,重新检测并计时 */
				}
				
				if( CH378GetDiskStatus( ) >= DEF_DISK_MOUNTED && i >= 5 ) 
				{
						break;  										 /* 有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS */
				}
		}
		if( s == ERR_DISK_DISCON ) 
		{  
				/* 检测到断开,重新检测并计时 */
				//printf( "Device gone\n" );
				return RT_ERROR;
		}
		if( CH378GetDiskStatus( ) < DEF_DISK_MOUNTED ) 
		{  
				/* 未知USB设备,例如USB键盘、打印机等 */
				//printf( "Unknown device\n" );
				return RT_ERROR;
		}	
				
		tfcard_ready = RT_TRUE;

		
		return RT_EOK;
}
int AddTestIndex(const char* TestLoc,long time,float a, float b, const char* unit)
{
	//ReadAllTestIndex()
	//Open 
	//open TestIndex.bin
	//if testindex.bin is not there, create one
	//read all lines
	//add 
	return -1;
}
int AddValue(int TestIndex,long time, unsigned int value)
{
	//open file
	//write value
	//need to create a buffer
	return 0;
}
int EndTestIndex(int TestIndex)
{
	//close file
	//write buffer into file
	return 0;
}
int DeleteTestIndex(int TestIndex)
{
	//open testindex.bin
	//mark 0 on del line
	return 0;
}
int ReadAllTestIndex(struct TestIndexListStc ti)
{
	//open testindex.bin
	//read all lines
	//if del is 0, continue
	//refresh ti
	//字节写 几十k，
	return 0;
}
