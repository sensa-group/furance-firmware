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

#include "pcf8574.h"

void GPIO_init(void)
{
}

void GPIO_relayOn(uint8_t relay)
{
    PCF8574_setOutputPin(GPIO_PCF8574_ID, relay, 1);
}

void GPIO_relayOff(uint8_t relay)
{
    PCF8574_setOutputPin(GPIO_PCF8574_ID, relay, 0);
}
