# File name:        Flash
# Description:      Flash script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-09

#!/bin/bash

#avrdude -p atmega32u4 -c usbasp -e -U flash:w:build/pecka.hex:i
avrdude -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -e -U flash:w:build/pecka.hex:i
