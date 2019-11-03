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

static void _intToStr(long long number, char *str);

void UART_init(void)
{
    uint16_t baudRate = BAUD(BAUD_RATE);
    UBRR1H = (uint8_t)(baudRate >> 8);
    UBRR1L = (uint8_t)(baudRate & 0xFF);

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

void UART_writeIntegerString(long long value)
{
    char str[50];

    _intToStr(value, str);

    UART_writeString(str);
}

static void _intToStr(long long number, char *str)
{
    uint8_t n = 0;
    char tmp;

    if (number == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    if (number < 0)
    {
        str[0] = '-';
        str++;
        number *= -1;
    }

    while (number > 0)
    {
        str[n] = number % 10 + 0x30;
        number /= 10;
        n++;
    }

    str[n] = '\0';

    for (uint8_t i = 0; i < n / 2; i++)
    {
        tmp = str[i];
        str[i] = str[n - i - 1];
        str[n - i - 1] = tmp;
    }
}
