#include "cmd_process.h"
#include "stdio.h"
#include "hmi_user_uart.h"
#include <rtthread.h>
#include "usart4.h"
static uint16 current_screen_id = 0;//当前画面ID
static int32 test_value = 0;//测试值
static uint8 update_en = 0;//更新标记
uint8 cmd_buffer[CMD_MAX_SIZE];//指令buffer
uint32 timer_tick_last_update = 0; //上一次更新的时间
extern volatile uint32  timer_tick_count; //定时器节拍
enum enum_DISPSTAT{DISPSTAT_MA,DISPSTAT_PROC} dispstat;
int rangeMax=500;
int rangeMin=0;
float slop=1;
float offset=2;
rt_event_t evt_record;
rt_event_t evt_readhistory;
struct rt_messagequeue mq_record;
char msg_pool[2048];
char unitBuf[10]={0x6d,0x67,0x2f,0x6d,0x5e,0x33,0x00};
void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	uchar buffer[12] = {0};
	sprintf(buffer,"%ld",value); //把整数转换为字符串
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(uint16 screen_id, uint16 control_id,float value)
{
	uchar buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//把浮点数转换为字符串(保留一位小数)
	SetTextValue(screen_id,control_id,buffer);
}
void AddDataToChannel(uint8 *buf)
{
	char tempbuf[4];
	memcpy(tempbuf,buf,4);
	rt_uint32_t event;
	if(rt_event_recv(evt_record,
									btn_DISPMA|btn_DISPPRC,
									RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
									RT_WAITING_NO,
									&event)==RT_EOK)
	{

		//if(event&btn_DISPMA)
//		{
//			dispstat=DISPSTAT_MA;
//		}
//		if(event&btn_DISPPRC)
//		{
//			dispstat=DISPSTAT_PROC;
//		}
	}

		float mAval_float;
		sscanf(tempbuf,"%f",&mAval_float);
	mAval_float=slop*mAval_float+0;
		uint8 out[4];
		float pAval_float;
		switch(dispstat)
		{
			case DISPSTAT_MA:
				SetTextValueFloat(recordscreen,15,mAval_float);
			HistoAddFloat(historyscreen,7,mAval_float);
			usart4.printf("V:%f",mAval_float);
			//GraphChannelDataAdd(recordscreen,1,0,tempbuf,4);
				break;
			case DISPSTAT_PROC:
				pAval_float = (mAval_float-4)*(rangeMax-rangeMin)/16+rangeMin;
				SetTextValueFloat(recordscreen,15,pAval_float);
			HistoAddFloat(recordscreen,1,pAval_float);
				break;
			
		}
		
		//GraphChannelDataAdd(recordscreen,1,0,out,2);
		//HistoDataAdd(recordscreen,1,out,4);
		
}

void th_ScreenProcessFunc()
{
	rt_mq_init(&mq_record,"mqt",&msg_pool[0],128-sizeof(void*),sizeof(msg_pool),RT_IPC_FLAG_FIFO);
	InitDisplayer();
	rt_err_t rst=0;
	while(1)
	{
		unsigned char buf[4];
		GetMessage();
	/****************************************************************************************************************
		特别注意
		MCU不要频繁向串口屏发送数据，否则串口屏的内部缓存区会满，从而导致数据丢失(缓冲区大小：标准型8K，基本型4.7K)
		1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
		2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。
		******************************************************************************************************************/

		//TODO: 添加用户代码
		//数据有更新时，每100毫秒刷新一次
	/*
		if(update_en&&timer_tick_count-timer_tick_last_update>=TIME_100MS)
		{
			update_en = 0;
			timer_tick_last_update = timer_tick_count;			

			UpdateUI();
		}
*/
		//这里为了演示方便，每100毫秒自动更新一次
		if(timer_tick_count-timer_tick_last_update>=TIME_100MS)
		{
			timer_tick_last_update = timer_tick_count;
			rst=rt_mq_recv(&mq_record,&buf[0],4,RT_WAITING_NO);
			if(rst==RT_EOK)
			{
				AddDataToChannel(buf);
				//添加数据到通道0
				//write to tf card
				//ad->tfcard->displayer
				//          ->network
			}
			UpdateUI();
			
		}
			rt_thread_delay(50);
	}
		
}

void starttest()
{
	uint16 i = 0;
		uint8 dat[100] = {0};

		//生成方波
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(recordscreen,1,0,dat,100);//添加数据到通道0
}

void GetMessage()
{
	unsigned short size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令        
		if(size>0)//接收到指令
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
		}
	}		
/*! 
 *  \brief  消息处理流程
 *  \param msg 待处理消息
 *  \param size 消息长度
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//指令类型
	uint8 ctrl_msg = msg->ctrl_msg;   //消息的类型
	uint8 control_type = msg->control_type;//控件类型
	uint16 screen_id = PTR2U16(&msg->screen_id);//画面ID
	uint16 control_id = PTR2U16(&msg->control_id);//控件ID
	uint32 value = PTR2U32(msg->param);//数值

	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://触摸屏按下
	case NOTIFY_TOUCH_RELEASE://触摸屏松开
		NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://写FLASH成功
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://写FLASH失败
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://读取FLASH成功
		NotifyReadFlash(1,cmd_buffer+2,size-6);//去除帧头帧尾
		break;
	case NOTIFY_READ_FLASH_FAILD://读取FLASH失败
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://读取RTC时间
		NotifyReadRTC(cmd_buffer[1],cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//画面ID变化通知
			{
				NotifyScreen(screen_id);
			}
			else
			{
				switch(control_type)
				{
				case kCtrlButton: //按钮控件
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://文本控件
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //进度条控件
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //滑动条控件
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //仪表控件
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://菜单控件
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://选择控件
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://倒计时控件
					NotifyTimer(screen_id,control_id);
					break;
				default:
					break;
				}
			}			
		}
		break;
	default:
		break;
	}
}

/*! 
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
void NotifyScreen(uint16 screen_id)
{
	//TODO: 添加用户代码
	
	current_screen_id = screen_id;//在工程配置中开启画面切换通知，记录当前画面ID
/*
	if(current_screen_id==4)//温度曲线
	{
		uint16 i = 0;
		uint8 dat[100] = {0};

		//生成方波
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//添加数据到通道0

		//生成锯齿波
		for (i=0;i<100;++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//添加数据到通道1
	}
	else if(current_screen_id==9)//二维码
	{
		//二维码控件显示中文字符时，需要转换为UTF8编码，
		//通过“指令助手”，转换“广州大彩123” ，得到字符串编码如下
		uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
	*/
}

/*! 
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
void NotifyTouchXY(uint8 press,uint16 x,uint16 y)
{
	//TODO: 添加用户代码
}



void UpdateUI()
{
	/*
	if(current_screen_id==2)//文本设置和显示
	{
		//当前电流、温度从0到100循环显示，艺术字从0.0-99.9循环显示
		SetTextValueInt32(2,5,test_value%100);//当前电流
		SetTextValueInt32(2,6,test_value%100);//温度
		SetTextValueFloat(2,7,(test_value%1000)/10.0);//艺术字

		++test_value;
	}
	else if(current_screen_id==5)//进度条和滑块控制
	{
		SetProgressValue(5,1,test_value%100);

		++test_value;
	}
	else if(current_screen_id==6)//仪表控件
	{
		SetMeterValue(6,1,test_value%360);
		SetMeterValue(6,2,test_value%360);

		++test_value;
	}
	*/
}

/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	//TODO: 添加用户代码
	rt_uint32_t event;
	switch(screen_id)
	{
		case mainscreen:
			
			break;
		case recordscreen:
		{
			switch(control_id)
			{
				case 2:
					//save button
				rt_event_send(evt_record,btn_SAVE);
				
					break;
				case 3:
					rt_event_send(evt_record,btn_START);
						ResetScreen();
				SetScreen(recordscreen);
					dispstat=DISPSTAT_MA;
				rt_event_send(evt_record,btn_DISPMA);
						ResetScreen();
				rt_thread_delay(20);
				SetScreen(recordscreen);
				SetTextValue(recordscreen,14,"mA");
				HistoRangeSet(recordscreen,1,20,0);
				HistoEnable(recordscreen,1);
					break;
				case 4:
					rt_event_send(evt_record,btn_START);
				ResetScreen();
				rt_thread_delay(20);
				SetScreen(recordscreen);
				HistoEnable(recordscreen,1);
				dispstat=DISPSTAT_PROC;
				SetTextValue(recordscreen,14,unitBuf);
				rt_event_send(evt_record,btn_DISPPRC);
				HistoRangeSet(recordscreen,1,rangeMax,rangeMin);

					break;
				case 5:
				//starttest();
				rt_event_send(evt_record,btn_START);
						ResetScreen();
				rt_thread_delay(20);
				SetScreen(recordscreen);
				HistoEnable(recordscreen,1);
				break;
				case 6:
					rt_event_recv(evt_record,
									btn_START,
									RT_EVENT_FLAG_AND|RT_EVENT_FLAG_CLEAR,
									RT_WAITING_NO,
									&event);
					rt_event_send(evt_record,btn_STOP);
				HistoDisable(recordscreen,1);
				
				break;
				case 7:
					if(rt_event_recv(evt_record,
									btn_START,
									RT_EVENT_FLAG_AND,
									RT_WAITING_NO,
									&event)==RT_EOK)
					{
						//please stop frist
					}
				//	stoptest();
				break;
				default:
					break;
			}
			break;
			}
		case historyscreen:
			break;
		default:
			break;
	}
	/*
	if(screen_id==3)//按钮、图标、动画控制
	{
		if(control_id==3)//运行按钮
		{
			if(state==0)//停止运行
			{				
				AnimationPlayFrame(3,1,1);//显示停止图标
				AnimationStop(3,2);//动画停止播放
			}
			else//开始运行
			{				
				SetControlVisiable(3,1,1);//显示图标
				SetControlVisiable(3,2,1);//显示动画

				AnimationPlayFrame(3,1,0);//显示运行图标
				AnimationStart(3,2);//动画开始播放
			}		
		}
		else if(control_id==4)//复位按钮
		{
			SetControlVisiable(3,1,0);//隐藏图标
			SetControlVisiable(3,2,0);//隐藏动画
			SetButtonValue(3,3,0);//显示开始运行
		}
	}
	*/
}
	

/*! 
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
	//TODO: 添加用户代码
	int32 value = 0; 

	if(screen_id==recordscreen)
	{
		
		switch(control_id)
		{
			case 8:
				//locA
				break;
			case 9:
				//loc b
				break;
			case 10:
				//loc c
				break;
			case 11:
				sscanf(str,"%s",unitBuf);
				//unit
				break;
			case 12:
				//range max
			sscanf(str,"%ld",&rangeMax);//把字符串转换为整数
				break;
			case 13:
				//range min
			sscanf(str,"%ld",&rangeMin);//把字符串转换为整数
				break;
		}
		
		/*
		if(control_id==1)
		{
			//限定数值范围（也可以在文本控件属性中设置）
			if(value<0)
				value = 0;
			else if(value>380)
				value = 380;

			SetTextValueInt32(2,1,value);  //更新最高电压
			SetTextValueInt32(2,4,value/2);  //更新最高电压/2
		}*/
	}
}

/*! 
 *  \brief  进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
	if(screen_id==5&&control_id==2)//滑块控制
	{
		test_value = value;

		SetProgressValue(5,1,test_value); //更新进度条数值
	}
}

/*! 
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
	//TODO: 添加用户代码
}

	/*! 
	 *  \brief  读取用户FLASH状态返回
	 *  \param status 0失败，1成功
	 *  \param _data 返回数据
	 *  \param length 数据长度
	 */
	void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
void NotifyWriteFlash(uint8 status)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取RTC时间，注意返回的是BCD码
 *  \param year 年（BCD）
 *  \param month 月（BCD）
 *  \param week 星期（BCD）
 *  \param day 日（BCD）
 *  \param hour 时（BCD）
 *  \param minute 分（BCD）
 *  \param second 秒（BCD）
 */
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second)
{
}
