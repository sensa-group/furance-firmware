/*
 * File name:       max6675.h
 * Description:     MAX6675 sensor module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-30
 * 
 */

#include "system.h"
#include "driver/max6675.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "driver/uart.h"

// library: https://github.com/adafruit/MAX6675-library/blob/master/max6675.cpp

static uint8_t _MAX6675_spiread(void);

void MAX6675_init(void)
{
    MAX6675_CS_DDR |= (1 << MAX6675_CS_PIN);
    MAX6675_SCK_DDR |= (1 << MAX6675_SCK_PIN);
    MAX6675_MISO_DDR &= ~(1 << MAX6675_MISO_PIN);

    MAX6675_CS_PORT |= (1 << MAX6675_CS_PIN);
}

double MAX6675_readCelsius(void)
{
    uint16_t v;

    MAX6675_CS_PORT &= ~(1 << MAX6675_CS_PIN);
    _delay_ms(1);

    v = _MAX6675_spiread();
    v <<= 8;
    v |= _MAX6675_spiread();

    MAX6675_CS_PORT |= (1 << MAX6675_CS_PIN);

    if (v & 0x4)
    {
        // uh oh, no thermocouple attached!
        return -100; 
        //return -100;
    }

    v >>= 3;

    return v * 0.25;
}

double MAX6675_readFahrenheit(void)
{
    return MAX6675_readCelsius() * 9.0 / 5.0 + 32;
}

static uint8_t _MAX6675_spiread(void)
{
    int i;
    uint8_t d = 0;

    for (i=7; i>=0; i--)
    {
        MAX6675_SCK_PORT &= ~(1 << MAX6675_SCK_PIN);
        _delay_ms(1);
        if (MAX6675_MISO_PORT & (1 << MAX6675_MISO_PIN))
        {
            //set the bit to 0 no matter what
            d |= (1 << i);
            //UART_writeString("JEEE! :D\n");
        }
        else
        {
            //UART_writeString("NEEE! ;(\n");
        }

        MAX6675_SCK_PORT |= (1 << MAX6675_SCK_PIN);
        _delay_ms(1);
    }

    return d;
}
