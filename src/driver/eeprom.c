/*
 * File name:       eeprom.c
 * Description:     EEPROM module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-28
 * 
 */

#include "system.h"
#include "driver/eeprom.h"

#include <avr/eeprom.h>

#include "driver/uart.h"

uint16_t EEPROM_readWord(uint16_t addr)
{
    return eeprom_read_word((uint16_t *)addr);
}

void EEPROM_writeWord(uint16_t addr, uint16_t value)
{
    eeprom_write_word((uint16_t *)addr, value);
}

