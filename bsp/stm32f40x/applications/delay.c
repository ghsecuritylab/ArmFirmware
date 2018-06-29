//#include <stm32f4xx.h>
#include "stm32f4xx.h"
#include "delay.h"

void delay_us_for_oneWire(rt_uint32_t nus)
{
    rt_uint32_t ticks;
    rt_uint32_t tick_old, tick_now, tick_cnt = 0;
    rt_uint32_t reload = SysTick->LOAD;                     //LOAD的值

    tick_cnt = 0;
		//ticks = nus * (SystemCoreClock / 1000000);      //需要的节拍数
		ticks = nus * (SystemCoreClock / 1000000 / 10);      //需要的节拍数
	
    rt_enter_critical();                                                    //调度器上锁，防止打断us延时

    tick_old = SysTick->VAL;                                            //刚进入时的计数器值
    while (1)
    {
        tick_now = SysTick->VAL;

        if (tick_now != tick_old)                                       //SYSTICK为递减计数器
        {
            if (tick_now < tick_old)
            {
                tick_cnt += tick_old - tick_now;
            }
            else
            {
                tick_cnt += reload - tick_now + tick_old;
            }

            tick_old = tick_now;

            if (tick_cnt >= ticks)                                      //达到延时时间
            {
                break;
            }
        }
    }

    rt_exit_critical();                                                 //关闭调度器锁
}

void delay_us(rt_uint32_t nus)
{
    rt_uint32_t ticks;
    rt_uint32_t tick_old, tick_now, tick_cnt = 0;
    rt_uint32_t reload = SysTick->LOAD;                     //LOAD的值

    tick_cnt = 0;
		ticks = nus * (SystemCoreClock / 1000000);      //需要的节拍数

    rt_enter_critical();                                                    //调度器上锁，防止打断us延时

    tick_old = SysTick->VAL;                                            //刚进入时的计数器值
    while (1)
    {
        tick_now = SysTick->VAL;

        if (tick_now != tick_old)                                       //SYSTICK为递减计数器
        {
            if (tick_now < tick_old)
            {
                tick_cnt += tick_old - tick_now;
            }
            else
            {
                tick_cnt += reload - tick_now + tick_old;
            }

            tick_old = tick_now;

            if (tick_cnt >= ticks)                                      //达到延时时间
            {
                break;
            }
        }
    }

    rt_exit_critical();                                                 //关闭调度器锁
}

void delay_ms(rt_uint32_t nms)
{
    rt_uint32_t fac_ms = 1000 / RT_TICK_PER_SECOND;

    if (nms >= fac_ms)
    {
        rt_thread_delay(nms / fac_ms);                      			//RT-Thread延时
    }

    nms %= fac_ms;                                                      //RT-Thread无法提供小于系统时钟节拍的延时

    delay_us((rt_uint32_t)(nms * 1000));
}
