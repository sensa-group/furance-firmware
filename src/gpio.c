/*
 * File name:       gpio.c
 * Description:     Module for digital input/output
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#include "system.h"
#include "gpio.h"

#include <avr/io.h>

#include "driver/pcf8574.h"
#include "driver/uart.h"
#include "driver/pwm.h"

void GPIO_init(void)
{
    // set to input
    GPIO_SWITCH_DDR &= ~((1 << GPIO_SWITCH_THERM) | (1 << GPIO_SWITCH_NIVO) | (1 << GPIO_SWITCH_SIGUR) | (1 << GPIO_SWITCH_OPC_1) | (1 << GPIO_SWITCH_OPC_2));
    // setup timer for debouncing?

    // buzzer
    GPIO_BUZZER_DDR |= (1 << GPIO_BUZZER_PIN);
    GPIO_BUZZER_PORT &= ~(1 << GPIO_BUZZER_PIN);

    GPIO_relayOff(GPIO_RELAY_HEATER);
}

void GPIO_buzzerOn(void) 
{
    //GPIO_BUZZER_PORT |= (1 << GPIO_BUZZER_PIN);
    PWM2_setFrequency(1);
}

void GPIO_buzzerOff(void) 
{
    //GPIO_BUZZER_PORT &= ~(1 << GPIO_BUZZER_PIN);
    PWM2_setFrequency(0);
}

uint8_t GPIO_switchRead(uint8_t switch_num) 
{
    // for switch_num just put one of the GPIO_SWITCH_ defines
    return !((GPIO_SWITCH_PORT >> switch_num) & 0x01);   
}

void GPIO_relayOn(uint8_t relay)
{
    pcf8574_setoutputpin(GPIO_PCF8574_ID, relay, 1);
}

void GPIO_relayOff(uint8_t relay)
{
    pcf8574_setoutputpin(GPIO_PCF8574_ID, relay, 0);
}
