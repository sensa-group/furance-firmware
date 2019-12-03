/*
 * File name:       uart.c
 * Description:     UART Definition
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

#define BAUD(baud) (F_CPU / 16 / baud - 1)

#define UART_RX_BUFFER_SIZE             255

#define UART_ESC            0x1B
#define UART_STX            0x02
#define UART_ETX            0x03

typedef void (*ptrReceiveCallback)(uint8_t *buffer, uint8_t bufferSize);

void UART_init(void);
uint8_t UART_read(void);
void UART_write(uint8_t data);
void UART_writeString(const char *str);
void UART_writeBuffer(uint8_t *buff, uint16_t len);
void UART_writeIntegerString(long long value);
void UART_setReaceiveCallback(ptrReceiveCallback receiveCallback);
uint8_t UART_recevingInProgress(void);

#endif // _UART_H_
