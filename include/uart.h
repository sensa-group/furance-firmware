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

void UART_init(void);
uint8_t UART_read(void);
void UART_write(uint8_t data);
void UART_writeString(char *str);
void UART_writeBuffer(uint8_t *buff, uint16_t len);

#endif // _UART_H_
