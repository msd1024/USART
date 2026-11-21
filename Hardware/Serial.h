#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include "stm32f10x.h"                  // Device header

#define RX_BUF_SIZE 256
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);

uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);




void USART1_Config(void);
typedef enum
{
    UART_OK      = 0,
    UART_TIMEOUT = 1,
    UART_ERROR   = 2,
    UART_BUSY    = 3
} UART_StatusTypeDef;

UART_StatusTypeDef UART_ReceiveToIdle(USART_TypeDef *USARTx,
                                      uint8_t *pData,
                                      uint16_t Size,
                                      uint16_t *RxLen,
                                      uint32_t Timeout);//返回值是枚举类型的串口IDLE接收函数


void USART1_DMA_Init(uint32_t baudrate);

#endif
