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
#define __AVR_ATmega32U4__
//#define __AVR_ATmega328P__

#define F_CPU 16000000UL

#define BAUD_RATE 9600
#define I2C_SCL_CLOCK 10000L

#endif // _SYSTEM_H_
