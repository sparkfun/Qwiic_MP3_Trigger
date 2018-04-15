#ifndef _Uart_H_
#define _Uart_H_

#include "typedef.h"
#include "MCU51.h"

#define Fosc       11059200L        /*¾§Õñ*/
#define Baudrate   9600           /*²¨ÌØÂÊ*/

//#define T1_TimerReload        250 //(256 - MAIN_Fosc / 192 / Baudrate0)       //Calculate the timer1 reload value   at 12T mode
#define T1_TimerReload        (256 - Fosc / 16 / Baudrate)      /*Calculate the timer1 reload value ar 1T mode  */
#define TimeOut    (28800 / (unsigned long)Baudrate + 2)


#define Self_Define_ISP_Download_Command 0x3D




void  Timer1_Init(void);
void  UartInit(void);
void  UartPutByte(u8 Uc_Data);
void  UartPutStr(u8 * Uc_Str);



#endif /* _Uart_H_ */

