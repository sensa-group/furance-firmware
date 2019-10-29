/* 
 * File:   DS18B20.c
 * Author: zi
 *
 * Created on October 2, 2019, 3:30 AM
 */

#include "system.h"
#include "driver/ds18b20o.h"

#include <avr/io.h>
#include <util/delay.h>

// OneWire commands
#define STARTCONVO      0x44  // Tells device to take a temperature reading and put it on the scratchpad
#define COPYSCRATCH     0x48  // Copy EEPROM
#define READSCRATCH     0xBE  // Read EEPROM
#define WRITESCRATCH    0x4E  // Write to EEPROM
#define RECALLSCRATCH   0xB8  // Reload from last known
#define READPOWERSUPPLY 0xB4  // Determine if device needs parasite power
#define ALARMSEARCH     0xEC  // Query bus for devices with an alarm condition

// Scratchpad locations
#define TEMP_LSB        0
#define TEMP_MSB        1
#define HIGH_ALARM_TEMP 2
#define LOW_ALARM_TEMP  3
#define CONFIGURATION   4
#define INTERNAL_BYTE   5
#define COUNT_REMAIN    6
#define COUNT_PER_C     7
#define SCRATCHPAD_CRC  8

// Device resolution
#define TEMP_9_BIT  0x1F //  9 bit
#define TEMP_10_BIT 0x3F // 10 bit
#define TEMP_11_BIT 0x5F // 11 bit
#define TEMP_12_BIT 0x7F // 12 bit

static uint8_t g_devices;
static uint8_t g_ds18Count;
static uint8_t g_parasite;
static uint8_t g_bitResolution;
static uint8_t g_waitForConversion;
static uint8_t g_checkForConversion;

static uint8_t _isValidAddress(uint8_t *addr);
static uint8_t _isValidFamilly(uint8_t *addr);
static uint8_t _getResolution(uint8_t *addr);
static uint8_t _readPowerSupply(uint8_t *addr);
static uint8_t _readScratchPad(uint8_t *addr, uint8_t *scratchPad);
static void _writeScratchPad(uint8_t *addr, uint8_t *scratchPad);

void writeByte(uint8_t data);

void resetDS()
{
    DDRE |= (1 << PE6);
    PORTE &= ~(1 << PE6);
    _delay_us(255);//480u reqd
    DDRE &= ~(1 << PE6);
    _delay_us(255);//480u reqd
    writeByte(0xCC);//ROM skip as only one sensor
}

void writeByte(uint8_t data){
    DDRE |= (1 << PE6);
  for (uint8_t mask = 00000001; mask>0; mask <<= 1){
    if (data & mask ){ // send 1
        PORTE &= ~(1 << PE6);
        _delay_us(10);//15u reqd
        PORTE |= (1 << PE6);
      _delay_us(45);//60u total
    }
    else{ //if bitwise and resolves to false
        PORTE &= ~(1 << PE6);
        _delay_us(50);
        PORTE |= (1 << PE6);
      //delayMicroseconds(5);//60u total is slow enough to negate needing this
    }
  }
}

uint8_t readByte(){
  uint8_t r = 0;
  for (uint8_t mask = 00000001; mask>0; mask <<= 1){
      DDRE |= (1 << PE6);
      PORTE &= ~(1 << PE6);
    //delayMicroseconds(3);//1u reqd pinMode() is slow enough to negate needing this
      PORTE |= (1 << PE6);
      _delay_us(10);
      if (PINE & (1 << PE6))
      {
          r |= mask;
      }
      _delay_us(40);
  }
  return r;
}

void DS18B20O_init(void)
{
}

double DS18B20O_readT1(void)
{
    resetDS();
    writeByte(0x44); // start conversion, without parasite power on at the end

    _delay_ms(1);

    resetDS();
    _delay_ms(1);
    writeByte(0xBE); // Read Scratchpad

    //uint16_t tmpInt = readByte();
    //_delay_ms(10);
    //tmpInt |= (readByte() << 8);
    double tmp = (readByte()|(readByte() << 8))*0.1125+32;//convert to deg f
    //double tmp = tmpInt * 0.1125 + 32;

    //return (readByte()|(readByte() << 8))*0.1125+32;//convert to deg f

    return (tmp - 32) * 5.0 / 9.0;
}

double DS18B20O_readT2(void)
{
}
