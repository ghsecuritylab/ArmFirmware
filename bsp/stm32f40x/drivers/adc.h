#ifndef __ADC_H__
#define __ADC_H__
#define ADC_REF 2.483
#define paramv3v3 2
#define paramvsense 7.667
//--------------------------- Define ---------------------------//

//-------------------------- Typedef----------------------------//
typedef  struct {
	int (* initialize)(void);
	int (* read)(int);
	float value[5];
}SYS_ADC_T;

extern SYS_ADC_T adc;

void th_readADFunc();

#endif