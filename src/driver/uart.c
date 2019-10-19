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
    UBRR0L = baudRate & 0xFF;
    UBRR0H = baudRate >> 8;

    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

uint8_t UART_read(void)
{
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void UART_write(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
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
