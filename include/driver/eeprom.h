/*
 * File name:       eeprom.h
 * Description:     EEPROM module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-28
 * 
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stdint.h>

#define EEPROM_ADDR_SYSTEM_RUNNING                          0x00

#define EEPROM_ADDR_GLOBAL_TEMP_MIN                         0x10
#define EEPROM_ADDR_GLOBAL_TEMP_MAX                         0x12
#define EEPROM_ADDR_GLOBAL_TEMP_CRITICAL                    0x14
#define EEPROM_ADDR_GLOBAL_TEMP_START                       0x18

#define EEPROM_ADDR_STARTING_FAN_TIME                       0x20
#define EEPROM_ADDR_STARTING_FAN_SPEED                      0x22
#define EEPROM_ADDR_STARTING_DISPENSER_TIME                 0x24
#define EEPROM_ADDR_STARTING_HEATER_TIME                    0x28
#define EEPROM_ADDR_STARTING_FAN2_WAITING_TIME              0x2A
#define EEPROM_ADDR_STARTING_FAN2_SPEED                     0x2C
#define EEPROM_ADDR_STARTING_FLAME_MIN                      0x2E
#define EEPROM_ADDR_STARTING_FLAME_TIME                     0x30

#define EEPROM_ADDR_STABILISATION_TOTAL_TIME                0x40
#define EEPROM_ADDR_STABILISATION_DISPENSER_TIME_ON         0x42
#define EEPROM_ADDR_STABILISATION_DISPENSER_TIME_OFF        0x44
#define EEPROM_ADDR_STABILISATION_FAN_SPEED                 0x48

#define EEPROM_ADDR_RUNNING_DISPENSER_TIME_ON               0x50
#define EEPROM_ADDR_RUNNING_DISPENSER_TIME_OFF              0x52
#define EEPROM_ADDR_RUNNING_FAN_SPEED                       0x54

#define EEPROM_ADDR_STOPPING_FAN_TIME                       0x60
#define EEPROM_ADDR_STOPPING_FAN_SPEED                      0x62
#define EEPROM_ADDR_STOPPING_FLAME_MAX                      0x64
#define EEPROM_ADDR_STOPPING_FLAME_TIME                     0x68

#define EEPROM_ADDR_FLAME_CALIBRATION                       0x70

uint16_t EEPROM_readWord(uint16_t add);
void EEPROM_writeWord(uint16_t addr, uint16_t value);

#endif // _EEPROM_H_
