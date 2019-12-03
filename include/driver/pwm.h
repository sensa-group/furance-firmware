/*
 * File name:       pwn.h
 * Description:     PWM Module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-23
 * 
 */

#ifndef _PWM_H_
#define _PWM_H_

#include <stdint.h>

void PWM0_init(void);
void PWM0_setDutyCycle(uint8_t dutyCycle);

void PWM1_init(void);
void PWM1_setFrequency(uint32_t frequency);

void PWM2_init(void);
void PWM2_setFrequency(uint32_t frequency);

uint32_t TIME_milis(void);
void TIME_reset(void);

#endif // _PWM_H_
