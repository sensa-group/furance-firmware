/*
 * File name:       pwn.c
 * Description:     PWM Module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-23
 * 
 */

#include "system.h"
#include "driver/pwm.h"

#include <avr/io.h>

void PWM0_init(void)
{
    DDRC |= (1 << PC6);

    TCNT3 = 0;
    OCR3A = 0;
    OCR3B = 0;
    OCR3C = 0;

    TCCR3A = (1 << COM3A1)  | (1 << WGM30);
    TCCR3B = (1<< CS30) | (1 << WGM32);

    TCNT3 = 0;
}

void PWM0_setDutyCycle(uint8_t dutyCycle)
{
    OCR3AL = dutyCycle;
}
