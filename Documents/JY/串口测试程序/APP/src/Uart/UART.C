#include "Uart.h "


/*****************************************************************************
 函 数 名  : putchar
 功能描述  : 串口发送一个字节数据
 输入参数  : unsigned char Uc_Data
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年3月22日
    作    者   : Blueice
    修改内容   : 新生成函数

*****************************************************************************/
void putchar ( u8 Uc_Data )
{
    if ( Uc_Data == '\n' )
    {
        UartPutByte ( 0x0d );
        UartPutByte ( 0x0a );
    }
    else
    {
        UartPutByte ( Uc_Data );
    }
    //  return (Uc_Data);
}
/*****************************************************************************
 函 数 名  : UartPutByte
 功能描述  : 串口发送一个字节数据
 输入参数  : unsigned char Uc_Data
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年3月22日
    作    者   : Blueice
    修改内容   : 新生成函数

*****************************************************************************/
void UartPutByte ( unsigned char Uc_Data )
{
    ES   = 0;
    TI   = 0;
    SBUF = Uc_Data;
    while ( !TI );
    TI   = 0;
    ES   = 1;
}
/*****************************************************************************
 函 数 名  : UartPutStr
 功能描述  : 串口发送字符串
 输入参数  : unsigned char *Uc_Str
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年3月22日
    作    者   : Blueice
    修改内容   : 新生成函数

*****************************************************************************/
void UartPutStr ( u8 *Uc_Str )
{
    while ( *Uc_Str )
    {
        UartPutByte ( *Uc_Str );
        Uc_Str++;
    }

    // UartPutByte ( 0x0D );
    //UartPutByte ( 0x0A );  /*结尾发送回车换行*/
}
/*****************************************************************************
 函 数 名  : ifdef MCU_AVR
 功能描述  : 串口接收中断
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年3月22日
    作    者   : Blueice
    修改内容   : 新生成函数

*****************************************************************************/

void Uart0_RX_ISR ( void ) interrupt 4
{

    if ( RI )
    {
        if ( SBUF == Self_Define_ISP_Download_Command )
        {
            IAP_CONTR = 0x60;
        }
        else
        {
            //SBUF = SBUF;
        }
        RI  = 0;
    }
}
/*****************************************************************************
 函 数 名  : UartInit
 功能描述  : 串口初使化
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年3月22日
    作    者   : Blueice
    修改内容   : 新生成函数

*****************************************************************************/
void UartInit ( void )
{
    PCON |= 0x80;                   /*UART0 Double Rate Enable  */
    SCON = 0x50;                    /*UART0 set as 8bit , UART0 RX enable  */
    TMOD &= ~ ( 1 << 6 );           /*Timer1 Set as Timer*/
    TMOD &= 0x0f;
    TMOD |= 0x20;                   /*Timer1 set as 8 bits auto relaod  */
    AUXR |=  ( 1 << 6 );            /*Timer1 set as 1T mode  */
    TH1 = T1_TimerReload;           /*Load the timer  */
    TR1  = 1;
    ES  = 1;
}
