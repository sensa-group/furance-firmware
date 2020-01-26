/*
 * File name:       adc.c
 * Description:     ADC module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-30
 * 
 */

#include "system.h"
#include "driver/adc.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "debug.h"

void ADC_init(void)
{
    DDRF &= ~(1 << PF7);

    //ADMUX = (1 << REFS0);

    //ADMUX = 0x00;
    ADMUX = (1 << REFS0);
    //ADMUX = (1 << REFS0) | (1 << REFS1);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    //DIDR0 = (1 << ADC0D);

    DDRB |= (1 << PB0);
    //PORTB |= (1 << PB0);
    PORTB &= ~(1 << PB0);

    //PORTB |= (1 << PB0);

    /*
    ADMUX = 0x00;
    ADCSRA = 0x00;
    DDRB |= (1 << PB0);
    PORTB &= ~(1 << PB0);
    */
}

uint8_t ADC_connected(uint8_t ch)
{
    uint8_t result = 1;

    return 1;

    if ((PINF >> PF7) & 0x01)
    {
        return 1;
    }

    return 0;

    int totalDiff = 0;

    for (uint8_t i = 0; i < 5; i++)
    {
        PORTB |= (1 << PB0);
        //PORTB &= ~(1 << PB0);

        _delay_ms(1);
        uint16_t value1 = ADC_read(ch);
        DEBUG_printf("ADC read 1 = %d\n", value1);

        if (value1 == 3)
        {
            result = 0;
            break;
        }

        PORTB &= ~(1 << PB0);
        //PORTB |= (1 << PB0);

        _delay_ms(1);
        uint16_t value2 = ADC_read(ch);
        DEBUG_printf("ADC read 2 = %d\n", value2);

        int diff = value1 - value2;
        if (diff < 0)
        {
            diff *= -1;
        }
        totalDiff += diff;
        DEBUG_printf("ADC read diff = %d\n\n", totalDiff);
    }

    if (totalDiff > 250)
    {
        result = 0;
    }

    return result;
}

uint16_t ADC_read(uint8_t ch)
{
    //ADCSRA |= (1 << ADEN);

    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8) | ch;     // clears the bottom 3 bits before ORing
 
    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1 << ADSC);
 
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1 << ADSC));

    //ADCSRA &= ~(1 << ADEN);

    uint16_t result = ADC;
 
    //return (ADCH << 8) | ADCL;
    return result;
}
