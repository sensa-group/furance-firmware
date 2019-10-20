/*
 * File name:       gpio.h
 * Description:     Module for digital input/output
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#include <avr/io.h>
#include <stdint.h>

// switches	
#define GPIO_SWITCH_DDR					DDRF
#define GPIO_SWITCH_PORT				PORTF
#define GPIO_SWITCH_THERM				6
#define GPIO_SWITCH_NIVO				5
#define GPIO_SWITCH_SIGUR				4
#define GPIO_SWITCH_OPC_1				1
#define GPIO_SWITCH_OPC_2				0

#define GPIO_BUZZER_DDR					DDRD
#define GPIO_BUZZER_PORT				PORTD
#define GPIO_BUZZER_PIN					4

#define GPIO_RELAY_MOTOR1               0
#define GPIO_RELAY_PUMP1                1
#define GPIO_RELAY_PUNP2                2
#define GPIO_RELAY_HEATER               3
#define GPIO_RELAY_AC_MOTOR1            4
#define GPIO_RELAY_MOTOR2               5
#define GPIO_RELAY_OPTIONAL             6

#define GPIO_PCF8574_ID                 1

void GPIO_init(void);

void GPII_switch_read(uint8_t switch_num);

void GPIO_relayOn(uint8_t relay);
void GPIO_relayOff(uint8_t relay);

void GPIO_buzzerOn(void);
void GPIO_buzzerOff(void);

#endif // _GPIO_H_
