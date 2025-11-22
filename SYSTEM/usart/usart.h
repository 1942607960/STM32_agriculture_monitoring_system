#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
	
#define BUFLEN 256      //数组缓存大小
typedef struct _UART_BUF
{
    char buf [BUFLEN+1];               
    unsigned int index ;
}UART_BUF;
	
	
void uart_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(u32 bound);
void UART1_send_byte(char data);
void UART2_send_byte(char data);
void UART3_send_byte(char data);
void Uart1_SendStr(char*SendBuf);
void Uart2_SendStr(char*SendBuf);
void Uart3_SendStr(char*SendBuf);

extern UART_BUF buf_uart1;     //CH340
extern UART_BUF buf_uart2;     //NBIOT
extern UART_BUF buf_uart3;     //TTL

#endif


