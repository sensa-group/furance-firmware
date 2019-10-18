/*
 * File name:       ds18b20.h
 * Description:     Module for DS18B20 temperature sensor
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-14
 * 
 */

#ifndef _DS18B20_H_
#define _DS18B20_H_

void DS18B20_init(void);
double DS18B20_readT1(void);
double DS18B20_readT2(void);

#endif // _DS18B20_H_
