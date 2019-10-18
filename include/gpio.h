/*
 * File name:       gpio.h
 * Description:     Module for digital input/output
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdint.h>

#define GPIO_RELAY_MOTOR1               0
#define GPIO_RELAY_PUMP1                1
#define GPIO_RELAY_PUNP2                2
#define GPIO_RELAY_HEATER               3
#define GPIO_RELAY_AC_MOTOR1            4
#define GPIO_RELAY_MOTOR2               5
#define GPIO_RELAY_OPTIONAL             6

#define GPIO_PCF8574_ID                 1

void GPIO_init(void);
void GPIO_relayOn(uint8_t relay);
void GPIO_relayOff(uint8_t relay);

#endif // _GPIO_H_
