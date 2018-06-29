#ifndef _PARA_H_
#define _PARA_H_

#include "ch378_inc.h"
#include "ch378_sys.h"

#define CH37x_INT_WIRE			PAin(3)

#define CH37x_CMD_PORT  *((volatile unsigned char  *)(0x64000001)) 	       /* CH375命令端口的I/O地址 */
#define CH37x_DAT_PORT	*((volatile unsigned char  *)(0x64000000)) 	       /* CH375数据端口的I/O地址 */


#define ERR_USB_UNKNOWN  0xFA

extern void init_para(void);
extern void CH37x_WR_CMD_PORT( unsigned char cmd );
extern void CH37x_WR_DAT_PORT( unsigned char dat );
extern unsigned char CH37x_RD_DAT_PORT(void);
extern void xEndCH37xCmd(void);

extern u8 mInitCH37xHost( void ) ;
extern u8 Query37xInterrupt( void );






#endif
