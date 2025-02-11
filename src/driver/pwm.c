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
#include <util/delay.h>

#include "driver/ds18b20.h"
#include "driver/adc.h"
#include "driver/uart.h"

static uint32_t _frequency;
static uint32_t _tick;

static uint32_t _frequency2;
static uint32_t _tick2;

static volatile double _temperatureSensorValue;
static volatile uint8_t _flameSensorValue;

static volatile uint64_t _time = 0;

int _freqOn = 0;
int _slowMode = 0;

void _isr1(void);
void _isr2(void);
void _readSensors(void);
void _refreshMenu(void);

void PWM0_init(void)
{
    cli();

    DDRC |= (1 << PC6);
    PORTC &= ~(1 << PC6);

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

    if (frequency == 0)
    {
        _frequency = 0;
    }
    else
    {
        if (frequency < 20)
        {
          uint16_t tmp = SYSTEM_MAP(frequency, 0, 100, 50, 700);
          _slowMode = 1;

          if (_frequency != tmp)
          {
              PORTB |= (1 << PB7);
              _delay_ms(1000);
              PORTB &= ~(1 << PB7);
          }

          _frequency = tmp;
        }
        else
        {
          _frequency = SYSTEM_MAP(frequency, 0, 100, 10, 70);
          _slowMode = 0;
        }
    }

    //_frequency = frequency;
    _tick = 0;
    TCNT1 = 0;

    if (frequency < 100)
    {
        PORTB &= ~(1 << PB7);
    }
    else
    {
        _frequency = 0;
        PORTB |= (1 << PB7);
    }

    sei();
}

void PWM2_init(void)
{
    cli();

    DDRD |= (1 << PD4);

    _frequency2 = 0;
    _tick2 = 0;

    sei();
}

void PWM2_setFrequency(uint32_t frequency)
{
    cli();

    _frequency2 = frequency;
    _tick2 = 0;
    TCNT1 = 0;

    PORTD &= ~(1 << PD4);

    sei();
}

uint32_t TIME_milis(void)
{
    return (uint32_t)_time;
}

void TIME_reset(void)
{
    _time = 0;
}

ISR(TIMER1_COMPA_vect)
{
    _time++;
    //_readSensors();
    //_refreshMenu();
    _isr1();
    _isr2();
}

void _readSensors(void)
{
    /*
    _temperatureSensorValue;
    _flameSensorValue;

    double temperatureValue = ds18b20_gettemp();
    uint8_t flameValue = ADC_read(0x07);

    if (_temperatureSensorValue != temperatureValue || _flameSensorValue != flameValue)
    {
        //MENU_refreshSensorValue((uint16_t)temperatureValue, (uint16_t)flameValue);
    }

    _temperatureSensorValue = temperatureValue;
    _flameSensorValue = flameValue;
    */
}

void _refreshMenu(void)
{
    //MENU_refresh();
}

void _isr1(void)
{
    if (_frequency == 0)
    {
        return;
    }

    if (_slowMode)
    {
        if (_freqOn)
        {
            _tick++;
            if (_tick > _frequency)
            {
                _freqOn = 0;
                PORTB &= ~(1 << PB7);
                _tick = 0;
            }
        }
        else
        {
            _tick++;
            if (_tick > 700)
            {
                _freqOn = 1;
                PORTB |= (1 << PB7);
                _tick = 0;
            }
        }
    }
    else
    {
        _tick++;
        if (_tick > _frequency)
        {
            PORTB ^= (1 << PB7);
            _tick = 0;
        }
    }
}

void _isr2(void)
{
    if (_frequency2 == 0)
    {
        return;
    }

    _tick2++;
    if (_tick2 > _frequency2)
    {
        PORTD ^= (1 << PD4);
        _tick2 = 0;
    }
}
