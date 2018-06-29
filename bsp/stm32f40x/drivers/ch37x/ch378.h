#ifndef _SPI_H_
#define _SPI_H_

#include "ch378_sys.h"
#include "ch378_inc.h"

#define CH37x_INT_WIRE			PAin(3)

#define SPI1_CS                 PAout(4)   

#define ERR_USB_UNKNOWN  0xFA

extern void CH37x_WR_CMD_PORT( unsigned char cmd );
extern void CH37x_WR_DAT_PORT( unsigned char dat );
extern unsigned char CH37x_RD_DAT_PORT(void);
extern void xEndCH37xCmd(void);

extern u8 mInitCH37xHost( void ) ;
extern u8 Query37xInterrupt( void );

void ch378_power_init(void);
void ch378_power_on(void);
void ch378_power_off(void);







#endif
