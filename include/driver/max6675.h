/*
 * File name:       max6675.h
 * Description:     MAX6675 sensor module
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-30
 * 
 */

#ifndef _MAX6675_H_
#define _MAX6675_H_

#include <avr/io.h>

#define MAX6675_SCK_DDR         DDRC
#define MAX6675_SCK_PORT        PORTC
#define MAX6675_SCK_PIN         PC7

#define MAX6675_MISO_DDR        DDRD
#define MAX6675_MISO_PORT       PIND
#define MAX6675_MISO_PIN        PD7

#define MAX6675_CS_DDR          DDRE
#define MAX6675_CS_PORT         PORTE
#define MAX6675_CS_PIN          PE2

void MAX6675_init(void);
double MAX6675_readCelsius(void);
double MAX6675_readFahrenheit(void);

#endif // _MAX6675_H_
