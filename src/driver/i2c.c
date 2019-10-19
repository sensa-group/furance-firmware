/*
 * File name:       i2c.c
 * Description:     I2C Implementation
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#include "system.h"
#include "driver/i2c.h"

#include <avr/io.h>

static void _I2C_start(void);
static void _I2C_stop(void);
static void _I2C_connect(uint8_t addr, uint8_t w);
static void _I2C_sendAddra(uint8_t addr, uint8_t w);

void I2C_init(void)
{
    TWBR = I2C_BAUD(I2C_SCL_CLOCK);
    TWSR = 0x00;
    TWCR = (1 << TWEN);
}

uint8_t I2C_read(uint8_t addr)
{
    _I2C_connect(addr, 0);

    TWCR = (1 << TWINT) | (1 << TWEN);
    //TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    uint8_t data = TWDR;

    _I2C_stop();

    return data;
}

void I2C_write(uint8_t addr, uint8_t data)
{
    _I2C_connect(addr, 1);

    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));

    _I2C_stop();
}

static void _I2C_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

static void _I2C_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

    while (TWCR & (1 << TWSTO));
}

static void _I2C_connect(uint8_t addr, uint8_t w)
{
    _I2C_start();
    _I2C_sendAddra(addr, w);
}

static void _I2C_sendAddra(uint8_t addr, uint8_t w)
{
    TWDR = (addr << 1) | w ? 0x01 : 0x00;
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}
