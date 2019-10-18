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
void DISPLAY_clear(void);
void DISPLAY_home(void);
void DISPLAY_gotoXY(uint8_t x, uint8_t y);
void DISPLAY_showString(const char *str);
void DISPLAY_showInteger(int number);

#endif // _DISPLAY_H_
