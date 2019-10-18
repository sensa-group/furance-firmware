/*
 * File name:       pcf8574.c
 * Description:     Driver for PCF8574 I2C GPIO expander
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#include "system.h"
#include "pcf8574.h"

#include <avr/io.h>

#include "i2c.h"

static uint8_t g_pinStatus[PCF8574_MAXDEVICES];

void PCF8574_init(void)
{
    for (uint8_t i = 0; i < PCF8574_MAXDEVICES; i++)
    {
        g_pinStatus[i] = 0x00;
    }
}

void PCF8574_setOutput(uint8_t deviceId, uint8_t data)
{
    if (deviceId >= PCF8574_MAXDEVICES)
    {
        return;
    }

    g_pinStatus[deviceId] = data;
    I2C_write(PCF8574_ADDRBASE + deviceId, g_pinStatus[deviceId]);
}

void PCF8574_setOutputPin(uint8_t deviceId, uint8_t pin, uint8_t data)
{
    if (deviceId >= PCF8574_MAXDEVICES || pin >= PCF8574_MAXPINS)
    {
        return;
    }

    if (data)
    {
        g_pinStatus[deviceId] |= (1 << pin);
    }
    else
    {
        g_pinStatus[deviceId] &= ~(1 << pin);
    }
    I2C_write(PCF8574_ADDRBASE + deviceId, g_pinStatus[deviceId]);
}

uint8_t PCF8574_getInput(uint8_t deviceId)
{
    if (deviceId >= PCF8574_MAXDEVICES)
    {
        return 0x00;
    }

    return I2C_read(PCF8574_ADDRBASE + deviceId);
}

uint8_t PCF8574_getInputPin(uint8_t deviceId, uint8_t pin)
{
    if (deviceId >= PCF8574_MAXDEVICES || pin >= PCF8574_MAXPINS)
    {
        return 0x00;
    }

    uint8_t data = I2C_read(PCF8574_ADDRBASE + deviceId);
    return (data >> pin) & 0x01;
}
