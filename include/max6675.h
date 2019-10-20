/*
 * File name:       max6675.h
 * Description:     Module for MAX6675 K type
 * Date:            2019-10-20
 * 
 */

#ifndef _MAX6675_H_
#define _MAX6675_H_

#include <avr/io.h>

#define PIN_SCK_DDR			DDRC
#define PIN_SCK_PORT		PORTC
#define PIN_SCK       		7

#define PIN_MISO_DDR		DDRD
#define PIN_MISO_PORT		PORTD
#define PIN_MISO       		7

#define PIN_CS_DDR 			DDRE
#define PIN_CS_PORT 		PORTE
#define PIN_CS        		2

void MAX6675_init(void);
double MAX6675_read_temperature(void);

#endif