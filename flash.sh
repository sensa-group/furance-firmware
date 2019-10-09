# File name:        Flash
# Description:      Flash script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-09

#!/bin/bash

avrdude -p atmega328p -c usbasp -e -U flash:w:build/pecke.hex:i
