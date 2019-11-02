/*
 * File name:       statemachine.h
 * Description:     Main logic
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-31
 * 
 */

#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include <stdint.h>

void SM_init(void);
void SM_exec(void);

void SM_start(void);
void SM_stop(void);
void SM_snailStart(void);
void SM_snailStop(void);

void SM_startStop(void);
void SM_snailStartStop(void);

void SM_showMenuItem(const char *description, const char *description2);
void SM_showMenuOption(uint16_t value);

#endif // _STATEMACHINE_H_
