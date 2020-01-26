/*
 * File name:       system.h
 * Description:     System configuration and constants
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-10
 * 
 */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/*
 * To make KDevelop happy :)
 */
//#define __AVR_ATmega32U4__
//#define __AVR_ATmega328P__

#define F_CPU 16000000UL
//#define F_CPU 12000000UL
//#define F_CPU 2000000UL
//#define F_CPU 8000000UL

#define BAUD_RATE 115200
//#define I2C_SCL_CLOCK 100000UL
//#define I2C_SCL_CLOCK 100000UL
//#define I2C_SCL_CLOCK 400000L
//#define SCL_CLOCK 100000L
#define SCL_CLOCK 10000L

#define SYSTEM_MAX(a, b) ( a > b ? a : b)
#define SYSTEM_MIN(a, b) ( a < b ? a : b)

#define SYSTEM_MAP(x, inMin, inMax, outMin, outMax) ((x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin)
//#define SYSTEM_MAP(input, input_start, input_end, output_start, output_end) (output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start))

#endif // _SYSTEM_H_
