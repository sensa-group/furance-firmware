# File name:        build.sh
# Description:      Build script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-08

#!/bin/bash

make clean && make
avr-objcopy -j .text -j .data -O ihex build/pecka.elf build/pecka.hex
