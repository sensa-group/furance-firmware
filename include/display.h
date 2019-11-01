/*
 * File name:       display.h
 * Description:     Module for display
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-14
 * 
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdint.h>

void DISPLAY_init(void);
uint8_t DISPLAY_clear(void);
uint8_t DISPLAY_home(void);
uint8_t DISPLAY_gotoXY(uint8_t x, uint8_t y);
uint8_t DISPLAY_showString(const char *str);
uint8_t DISPLAY_showInteger(int number);

#endif // _DISPLAY_H_
