/*
 * File name:       i2c.h
 * Description:     I2C Definition
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-13
 * 
 */

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

#define I2C_BAUD(scl_clk) (((F_CPU / scl_clk) - 16) / 2)

void I2C_init(void);
uint8_t I2C_read(uint8_t addr);
void I2C_write(uint8_t addr, uint8_t data);

#endif // _I2C_H_
