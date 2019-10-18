/*
 * File name:       main.c
 * Description:     Start point of firmware
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-09
 * 
 */

#include "system.h"

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB |= (1 << PB5);

    while (1)
    {
        PORTB ^= (1 << PB5);
        _delay_ms(1000);
    }

    return 0;
}
