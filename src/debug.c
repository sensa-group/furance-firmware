#include "system.h"
#include "debug.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdarg.h>

#include "driver/uart.h"
#include "driver/softuart.h"

#ifdef DEBUG_ENABLED
uint8_t _hex_map[] = { 'A', 'B', 'C', 'D', 'E', 'F' };
#endif

void _DEBUG_byteToString(uint8_t data, char *str);
void _DEBUG_intToString(long long number, char *str);

void DEBUG_init(void)
{
#ifdef DEBUG_ENABLED

#ifdef DEBUG_SOFTWARE
    uart_init();
#endif // DEBUG_SOFTWARE

#endif // DEBUG_ENABLED
}

void DEBUG_logString(const char *str)
{
#ifdef DEBUG_ENABLED

#ifdef DEBUG_HARDWARE
    UART_writeString(str);
#endif // DEBUG_ENABLED

#ifdef DEBUG_SOFTWARE
    cli();
    uart_tx_str(str);
    sei();
#endif // DEBUG_SOFTWARE

#endif // DEBUG_ENABLED
}

void DEBUG_logByte(uint8_t data)
{
#ifdef DEBUG_ENABLED

    char str[5];
    _DEBUG_byteToString(data, str);

#ifdef DEBUG_HARDWARE
    UART_writeString(str);
#endif // DEBUG_HARDWARE

#ifdef DEBUG_SOFTWARE
    cli();
    uart_tx_str(str);
    sei();
#endif // DEBUG_SOFTWARE

#endif // DEBUG_ENABLED
}

void DEBUG_logInteger(long long data)
{
#ifdef DEBUG_ENABLED

    char str[15];
    _DEBUG_intToString(data, str);

#ifdef DEBUG_HARDWARE
    UART_writeString(str);
#endif // DEBUG_HARDWARE

#ifdef DEBUG_SOFTWARE
    cli();
    uart_tx_str(str);
    sei();
#endif // DEBUG_SOFTWARE

#endif // DEBUG_ENABLED
}

void DEBUG_printf(const char *str, ...)
{
#ifdef DEBUG_ENABLED
    char buffer[DEBUG_BUFFER_SIZE];
    char tmpBuffer[10];
    strcpy(buffer, str);

    va_list args;
    va_start(args, str);

    char *tmp = buffer;
    char *start = buffer;

    for (; *tmp != '\0'; tmp++)
    {
        if (*tmp == '%')
        {
            *tmp = '\0';
            if (tmp != start)
            {
                DEBUG_logString(start);
            }

            switch (*(tmp + 1))
            {
                case 'b':
                    _DEBUG_byteToString((uint8_t)va_arg(args, int), tmpBuffer);
                    DEBUG_logString(tmpBuffer);
                    break;
                case 'd':
                    _DEBUG_intToString(va_arg(args, int), tmpBuffer);
                    DEBUG_logString(tmpBuffer);
                    break;
                case 's':
                    DEBUG_logString(va_arg(args, const char *));
                    break;
                default:
                    DEBUG_logByte(*(tmp + 1));
                    break;
            }

            tmp++;
            start = tmp + 1;
        }
    }

    if (tmp != start)
    {
        DEBUG_logString(start);
    }

    va_end(args);
#endif // DEBUG_ENABLED
}

void _DEBUG_byteToString(uint8_t data, char *str)
{
    uint8_t tmp;

    str[0] = '0';
    str[1] = 'x';

    tmp = (data >> 4) & 0x0F;
    if (tmp > 9)
    {
        tmp -= 10;
        str[2] = _hex_map[tmp];
    }
    else
    {
        str[2] = tmp + '0';
    }

    tmp = data & 0x0F;
    if (tmp > 9)
    {
        tmp -= 10;
        str[3] = _hex_map[tmp];
    }
    else
    {
        str[3] = tmp + '0';
    }

    str[4] = '\0';
}

void _DEBUG_intToString(long long number, char *str)
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
        str[n] = number % 10 + '0'; // + 0x30;
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
