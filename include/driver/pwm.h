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

#endif // _PWM_H_
