/*
 * File name:       max6675.c
 * Description:     Module for MAX6675 K type
 * Date:            2019-10-20
 * 
 */

#include "system.h"
#include "max6675.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

// library: https://github.com/adafruit/MAX6675-library/blob/master/max6675.cpp

static uint8_t spi_read(void) {
	/*
		int i;
	  byte d = 0;

	  for (i=7; i>=0; i--)
	  {
	    digitalWrite(sclk, LOW);
	    _delay_ms(1);
	    if (digitalRead(miso)) {
	      //set the bit to 0 no matter what
	      d |= (1 << i);
	    }

	    digitalWrite(sclk, HIGH);
	    _delay_ms(1);
	  }

	  return d;
	 */


	int counter;
	uint8_t read_data = 0;

	for(counter = 7; counter >= 0; counter--) {

		PIN_SCK_PORT &= ~(1 << PIN_SCK);

		_delay_ms(1);

		if(PIN_MISO_PORT & (1 << PIN_MISO)) { // high
			read_data |= (1 << counter);
		}

		PIN_SCK_PORT |= (1 << PIN_SCK);
	}

	return read_data;
}

static double read_temp(void) {

	/*
	uint16_t v;

	  digitalWrite(cs, LOW);
	  _delay_ms(1);

	  v = spiread();
	  v <<= 8;
	  v |= spiread();

	  digitalWrite(cs, HIGH);

	  if (v & 0x4) {
	    // uh oh, no thermocouple attached!
	    return NAN; 
	    //return -100;
	  }

	  v >>= 3;

	  return v*0.25;
	  */

	uint16_t temp_data;

	PIN_CS_PORT &= ~(1 << PIN_CS);
	_delay_ms(1);

	temp_data = spi_read();
	temp_data <<= 8;
	temp_data |= spi_read();

	PIN_CS_PORT |= (1 << PIN_CS);

	if(temp_data & 0x4) {
		// no thermocouple attached
		return -100;
	}

	temp_data >>= 3;
	return temp_data * 0.25;

}


void MAX6675_init(void) {

	PIN_SCK_DDR |= (1 << PIN_SCK);			// output
	PIN_CS_DDR |= (1 << PIN_CS);			// output
	PIN_MISO_DDR &= ~(1 << PIN_MISO);		// input

	// maybe set SCK and MISO to low? 

	PIN_CS_PORT |= (1 << PIN_CS);			// high
}

double MAX6675_read_temperature(void) {
	return read_temp() * 9.0/5.0 + 32;
}
