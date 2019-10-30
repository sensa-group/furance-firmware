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

void ADC_init(void)
{
    DDRF &= ~(1 << PF7);

    //ADMUX = (1 << REFS0);

    //ADMUX = 0x00;
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    //DIDR0 = (1 << ADC0D);
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
 
    //return (ADCH << 8) | ADCL;
    return ADC;
}
