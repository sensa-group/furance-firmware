/*
 * File name:       adc.h
 * Description:     ADC module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-30
 * 
 */

#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

void ADC_init(void);
uint8_t ADC_connected(uint8_t ch);
uint16_t ADC_read(uint8_t ch);

#endif // _ADC_H_
