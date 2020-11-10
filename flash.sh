# File name:        Flash
# Description:      Flash script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-09

#!/bin/bash

#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0xDE:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m
#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0xE2:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m
#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0xDE:m -U hfuse:w:0xD2:m -U efuse:w:0xFF:m           # External clock; Preserver EEPROM; Use in production
#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0xe2:m -U hfuse:w:0xd2:m -U efuse:w:0xff:m         	# Internal clock; Preserver EEPROM; Use in production (backup)
#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0x9e:m -U hfuse:w:0xd2:m -U efuse:w:0xff:m 	# Test
#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0xa2:m -U hfuse:w:0xd2:m -U efuse:w:0xff:m  # Test internal
avrdude -p atmega32u4 -c usbasp -e -U flash:w:build/pecka.hex:i -V
#avrdude -p atmega32u4 -c dragon_isp -U lfuse:w:0xDE:m -U hfuse:w:0xD2:m -U efuse:w:0xFF:m           # External clock; Preserver EEPROM; Use in production
#avrdude -p atmega32u4 -c dragon_isp -e -U flash:w:build/pecka.hex:i -V
#avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -e -U flash:w:build/pecka.hex:i
#avrdude -p atmega2560 -c wiring -P /dev/ttyUSB0 -b 115200 -D -U flash:w:build/pecka.hex:i
