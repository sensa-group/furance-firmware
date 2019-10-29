# File name:        Flash
# Description:      Flash script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-09

#!/bin/bash

#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0xDE:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m
#avrdude -p atmega32u4 -c usbasp -U lfuse:w:0xE2:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m
avrdude -p atmega32u4 -c usbasp -e -U flash:w:build/pecka.hex:i -V
#avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -e -U flash:w:build/pecka.hex:i
#avrdude -p atmega2560 -c wiring -P /dev/ttyUSB0 -b 115200 -D -U flash:w:build/pecka.hex:i
