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
#include <avr/interrupt.h>

static uint32_t _frequency;
static uint32_t _tick;

void PWM0_init(void)
{
    cli();

    DDRC |= (1 << PC6);

    TCNT3 = 0;
    OCR3A = 0;
    OCR3B = 0;
    OCR3C = 0;

    TCCR3A = (1 << COM3A1)  | (1 << WGM30);
    TCCR3B = (1<< CS30) | (1 << WGM32);

    TCNT3 = 0;

    sei();
}

void PWM0_setDutyCycle(uint8_t dutyCycle)
{
    OCR3AL = dutyCycle;
}

void PWM1_init(void)
{
    cli();

    DDRB |= (1 << PB7);

    TCNT1 = 0;
    OCR1A = 0;
    OCR1B = 0;
    OCR1C = 0;

    //TCCR1A = (1 << COM1A1);
    TCCR1B = (1<< CS10) | (1 << WGM12);
    TIMSK1 = (1 << OCIE1A);

    TCNT1 = 0;

    OCR1A = 16000;
    //OCR1A = 16;

    _frequency = 0;
    _tick = 0;

    sei();
}

void PWM1_setFrequency(uint32_t frequency)
{
    cli();

    _frequency = frequency;
    _tick = 0;
    TCNT1 = 0;

    PORTB &= ~(1 << PB7);

    sei();
}

ISR(TIMER1_COMPA_vect)
{
    if (_frequency == 0)
    {
        return;
    }

    _tick++;
    if (_tick > _frequency)
    {
        PORTB ^= (1 << PB7);
        _tick = 0;
    }
}
