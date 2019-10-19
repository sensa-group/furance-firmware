/*
 * File name:       uart.c
 * Description:     UART Implementation
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#include "system.h"
#include "driver/uart.h"

#include <avr/io.h>

void UART_init(void)
{
    uint16_t baudRate = BAUD(BAUD_RATE);
    UBRR1L = baudRate & 0xFF;
    UBRR1H = baudRate >> 8;

    UCSR1B = (1 << RXEN1) | (1 << TXEN1);

    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

uint8_t UART_read(void)
{
    while (!(UCSR1A & (1 << RXC1)));
    return UDR1;
}

void UART_write(uint8_t data)
{
    while (!(UCSR1A & (1 << UDRE1)));
    UDR1 = data;
}

void UART_writeString(char *str)
{
    while (*str)
    {
        UART_write(*str);
        str++;
    }
}

void UART_writeBuffer(uint8_t *buff, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        UART_write(buff[i]);
    }
}
