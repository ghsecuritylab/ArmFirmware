#ifndef __KEY_SCAN_H__
#define __KEY_SCAN_H__

#include <rtthread.h>

#define KEY_SET                     0
#define KEY_RESET                   1

#define NUM_OF_KEYS                 9

#define KEY_EVENT_TAG               0                   //PI.8	事件 
#define KEY_POWER                   1                   //PI.5  电源键
#define KEY_START                   2                   //PI.11	开始/停止
#define KEY_CONFIG                  3                   //PA.2	设置 SET
#define KEY_FORMAT                  4                   //PH.3	格式化 CANCELL
#define KEY_START_AND_CONFIG    	5                   //PI.11 & PA.2
#define KEY_START_AND_FORMAT    	6                  	//PI.11 & PH.3
#define KEY_CONFIG_AND_FORMAT   	7               	//PA.2 & PH.3
#define KEY_EVENT_AND_CONFIG   		8               	//PI.8 & PA.2


#define KEY_DEBOUNCE_COUNT      	1           		//按键消抖key_press_counter计数起始值
#define KEY_HOLD_FIRST_COUNT    	40          		//开始长按key_press_counter计数起始值
#define KEY_HOLD_INTERVAL           10          		//长按key_press_counter计数区间
#define KEY_HOLD_1S_COUNT           1           		//长按1s key_hold_counter计数值
#define KEY_HOLD_3S_COUNT           11          		//长按3s key_hold_counter计数值
														//(KEY_HOLD_1S_COUNT + 1000 / (KEY_HOLD_INTERVAL    *20) * (3 - 1))
#define KEY_RELEASE_1S_COUNT    	50              	//按键释放1s key_release_counter计数值

struct key_handler                              		//按键处理
{
    rt_uint8_t key_last_status;                 		//上一次扫描按键值
    rt_uint8_t key_status;                      		//当前扫描按键值
    rt_uint32_t key_press_counter;              		//按键按下计数器
    rt_uint32_t key_hold_counter;               		//按键长按计数器
    rt_uint32_t key_release_counter;            		//按键释放计数器
    void (*key_press_event)(void);              		//按键短按执行任务
    void (*key_hold_event)(void);               		//按键连按执行的任务
    void (*key_hold_1s_event)(void);            		//按键长按1s执行的任务
    void (*key_hold_3s_event)(void);            		//按键长按3s执行的任务
    void (*key_release_1s_event)(void);     			//按键释放3s执行的任务
};


void power_on(void);
void power_off(void);
void key_scan_init(void);
void key_handler_init(void);
void dynamic_acquisition_disable(void);

#endif
