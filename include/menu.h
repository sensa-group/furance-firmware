/*
 * File name:       menu.h
 * Description:     Module for menu logic
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-11-21
 * 
 */

#ifndef _MENU_H_
#define	_MENU_H_

#include <stdint.h>

void MENU_init(void);
void MENU_refreshSensorValue(uint16_t temperature, uint16_t flame);
void MENU_refreshError(const char *str1, const char *str2);

#endif /* _MENU_H_ */
