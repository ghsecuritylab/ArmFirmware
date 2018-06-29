#ifndef __DELAY_H__
#define __DELAY_H__

#include <rtthread.h>

void delay_us(rt_uint32_t nus);
void delay_us_for_oneWire(rt_uint32_t nus);
void delay_ms(rt_uint32_t nms);

#endif
