# File name:        build.sh
# Description:      Build script
# Author:           Milos Zivlak <milos.zivlak@sensa-group.net>
# Date:             2019-10-08

#!/bin/bash

make clean && make
strip --strip-debug build/pecka.elf
avr-size --mcu=atmega32u4 -C build/pecka.elf
avr-objcopy -j .text -j .data -O ihex build/pecka.elf build/pecka.hex
