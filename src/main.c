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

#include "driver/pwm.h"

int main(void)
{
    /*
    DDRC |= (1 << PC6);

    while (1)
    {
        PORTC ^= (1 << PC6);
        _delay_ms(1000);
    }
    */

    //PWM0_init();
    //PWM0_setDutyCycle(255);
    //PWM0_setDutyCycle(100);

    /*
    while (1)
    {
        for (uint16_t i = 100; i < 255; i += 25)
        {
            PWM0_setDutyCycle(i);
            _delay_ms(500);
        }
    }

    while (1);

    while (1)
    {
        PWM0_setDutyCycle(0);
        _delay_ms(1000);
        PWM0_setDutyCycle(0xFF / 4);
        _delay_ms(1000);
        PWM0_setDutyCycle(0xFF / 2);
        _delay_ms(1000);
        PWM0_setDutyCycle(0xFF);
        _delay_ms(1000);
    }
    */

    while (1);

    return 0;
}
