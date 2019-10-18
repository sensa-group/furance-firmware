/*
 * File name:       pcf8574.h
 * Description:     Driver for PCF8574 I2C GPIO expander
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#ifndef _PCF8574_H_
#define _PCF8574_H_

#include <stdint.h>

#define PCF8574_ADDRBASE            0x20

#define PCF8574_MAXDEVICES          2
#define PCF8574_MAXPINS             8

void PCF8574_init(void);
void PCF8574_setOutput(uint8_t deviceId, uint8_t data);
void PCF8574_setOutputPin(uint8_t deviceId, uint8_t pin, uint8_t data);
uint8_t PCF8574_getInput(uint8_t deviceId);
uint8_t PCF8574_getInputPin(uint8_t deviceId, uint8_t pin);

#endif // _PCF8574_H_
