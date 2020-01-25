/*
 * File name:       mcp7940.h
 * Description:     MCP7940 RTC
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-11-27
 * 
 */

#include "system.h"
#include "driver/mcp7940.h"

#include <avr/io.h>
#include <util/delay.h>

#include "driver/uart.h"
#include "driver/i2cmaster.h"

static uint8_t _bcd2int(uint8_t bcd);
static uint8_t _int2bcd(uint8_t dec);

static uint8_t _readByte(uint8_t addr);
static void _writeByte(uint8_t addr, uint8_t data);

static uint8_t _readRegisterBit(uint8_t reg, uint8_t b);
static void _setRegisterBit(uint8_t reg, uint8_t b);
static void _clearRegisterBit(uint8_t reg, uint8_t b);

void MCP7940_init(void)
{
    MCP7940_deviceStart();
    _setRegisterBit(MCP7940_RTCWKDAY, MCP7940_VBATEN);
    return;

    uint8_t crystalStatus = _readRegisterBit(MCP7940_RTCSEC, MCP7940_ST);
    while (!crystalStatus)
    {
        MCP7940_deviceStart();
        crystalStatus = _readRegisterBit(MCP7940_RTCSEC, MCP7940_ST);
        if (!crystalStatus)
        {
            _delay_ms(1000);
        }
    }
}

uint8_t MCP7940_deviceStart(void)
{
    _setRegisterBit(MCP7940_RTCSEC, MCP7940_ST);
    uint8_t status = 0;
    for (uint8_t i = 0; i < 255; i++)
    {
        status = _readRegisterBit(MCP7940_RTCWKDAY, MCP7940_OSCRUN);
        if (status) break;
        _delay_ms(1);
    }

    return status;
}

uint8_t MCP7940_deviceStop(void)
{
    _clearRegisterBit(MCP7940_RTCSEC, MCP7940_ST);
    uint8_t status = 0;
    for (uint8_t i = 0; i < 255; i++)
    {
        status = _readRegisterBit(MCP7940_RTCWKDAY, MCP7940_OSCRUN);
        if (!status) break;
        _delay_ms(1);
    }

    return status;
}

void MCP7940_now(uint8_t *ss, uint8_t *mm, uint8_t *hh, uint8_t *d, uint8_t *m, uint16_t *y)
{
    i2c_start((MCP7940_ADDRESS << 1) | I2C_WRITE);
    i2c_write(MCP7940_RTCSEC);
    i2c_stop();

    i2c_start((MCP7940_ADDRESS << 1) | I2C_READ);
    *ss = _bcd2int(i2c_readAck() & 0x7F);
    *mm = _bcd2int(i2c_readAck() & 0x7F);
    *hh = _bcd2int(i2c_readAck() & 0x3F);
    i2c_readAck();  // Ignore day of week
    *d = _bcd2int(i2c_readAck() & 0x3F);
    *m = _bcd2int(i2c_readAck() & 0x1F);
    *y = _bcd2int(i2c_readNak()) + 2000;
    i2c_stop();
}

void MCP7940_adjust(uint8_t ss, uint8_t mm, uint8_t hh, uint8_t d, uint8_t m, uint16_t y)
{
    MCP7940_deviceStop();

    _writeByte(MCP7940_RTCSEC, _int2bcd(ss));
    _writeByte(MCP7940_RTCMIN, _int2bcd(mm));
    _writeByte(MCP7940_RTCHOUR, _int2bcd(hh));
    // TODO: Weekday write
    _writeByte(MCP7940_RTCDATE, _int2bcd(d));
    _writeByte(MCP7940_RTCMTH, _int2bcd(m));
    _writeByte(MCP7940_RTCYEAR, _int2bcd(y - 2000));

    MCP7940_deviceStart();
}

uint8_t _bcd2int(uint8_t bcd)
{
    return ((bcd / 16 * 10) + (bcd % 16));
}

uint8_t _int2bcd(uint8_t dec)
{
    return ((dec / 10 * 16) + (dec % 10));
}


static uint8_t _readByte(uint8_t addr)
{
    i2c_start((MCP7940_ADDRESS << 1) | I2C_WRITE);
    i2c_write(addr);
    i2c_stop();
    i2c_start((MCP7940_ADDRESS << 1) | I2C_READ);
    i2c_write(addr);
    uint8_t data = i2c_readAck();
    i2c_stop();

    return data;
}

static void _writeByte(uint8_t addr, uint8_t data)
{
    i2c_start((MCP7940_ADDRESS << 1) | I2C_WRITE);
    i2c_write(addr);
    i2c_write(data);
    i2c_stop();
}

static uint8_t _readRegisterBit(uint8_t reg, uint8_t b)
{
    return (_readByte(reg) >> b) & 0x01;
}

static void _setRegisterBit(uint8_t reg, uint8_t b)
{
    _writeByte(reg, _readByte(reg) | (1 << b));
}

static void _clearRegisterBit(uint8_t reg, uint8_t b)
{
    _writeByte(reg, _readByte(reg) & ~(1 << b));
}
