/*
 * File name:       menu.c
 * Description:     Module for menu logic
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-28
 * 
 */

#include "system.h"
#include "menu.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "driver/uart.h"
#include "driver/eeprom.h"
#include "display.h"
#include "statemachine.h"

typedef void (*ptrStateFunction)(void);

/*
 * States
 */

#define _STATE_NO_MENU                   0
#define _STATE_START_STOP                1
#define _STATE_SNAIL                     2       // Punjenje puza
#define _STATE_TEMPERATURE               3
#define _STATE_STARTING                  4
#define _STATE_STABILISATION             5
#define _STATE_RUNNING                   6
#define _STATE_STOPPING                  7
#define _STATE_FLAME_SENSOR              8
#define _STATE_DATE_TIME                 9
#define _STATE_ADMINISTRATION           10
#define _STATE_EXIT                     11

#define _STATE_TEMPERATURE_MIN          12
#define _STATE_TEMPERATURE_MAX          13
#define _STATE_TEMPERATURE_EX           14

#define _STATE_TEMPERATURE_MIN_O        15
#define _STATE_TEMPERATURE_MAX_O        16

#define _STATE_START_FAN1_T             17
#define _STATE_START_FAN1_S             18
#define _STATE_START_DISPENSER          19
#define _STATE_START_HEATER_T           20
#define _STATE_START_FAN2_T             21
#define _STATE_START_FAN2_S             22
#define _STATE_START_EX                 23

#define _STATE_START_FAN1_T_O           24
#define _STATE_START_FAN1_S_O           25
#define _STATE_START_DISPENSER_O        26
#define _STATE_START_HEATER_T_O         27
#define _STATE_START_FAN2_T_O           28
#define _STATE_START_FAN2_S_O           29

#define _STATE_STAB_DISPENSER_ON        30
#define _STATE_STAB_DISPENSER_OFF       31
#define _STATE_STAB_FAN_S               32
#define _STATE_STAB_TOTAL_T             33
#define _STATE_STAB_EX                  34

#define _STATE_STAB_DISPENSER_ON_O      35
#define _STATE_STAB_DISPENSER_OFF_O     36
#define _STATE_STAB_FAN_S_O             37
#define _STATE_STAB_TOTAL_T_O           38

#define _STATE_RUN_DISPENSER_ON         39
#define _STATE_RUN_DISPENSER_OFF        40
#define _STATE_RUN_FAN_S                41
#define _STATE_RUN_EX                   42

#define _STATE_RUN_DISPENSER_ON_O       43
#define _STATE_RUN_DISPENSER_OFF_O      44
#define _STATE_RUN_FAN_S_O              45

#define _STATE_STOP_FAN_T               46
#define _STATE_STOP_FAN_S               47
#define _STATE_STOP_EX                  48

#define _STATE_STOP_FAN_T_O             49
#define _STATE_STOP_FAN_S_O             50

#define _STATE_FLAME_MIN                51
#define _STATE_FLAME_MIN_T              52
#define _STATE_FLAME_MAX                53
#define _STATE_FLAME_MAX_T              54
#define _STATE_FLAME_EX                 55

#define _STATE_FLAME_MIN_O              56
#define _STATE_FLAME_MIN_T_O            57
#define _STATE_FLAME_MAX_O              58
#define _STATE_FLAME_MAX_T_O            59

#define _STATE_ADMIN_TEMP_CRT           60
#define _STATE_ADMIN_TEMP_START         61
#define _STATE_ADMIN_EX                 62

#define _STATE_ADMIN_TEMP_CRT_O         63
#define _STATE_ADMIN_TEMP_START_O       64

/*
 * Results
 */

#define _TYPE_NO                         0
#define _TYPE_SUBMENU                    1
#define _TYPE_OPTION                     2
#define _TYPE_ACTION                     3

#define _REFRESH_DISPLAY_NO              0
#define _REFRESH_DISPLAY_INIT            1
#define _REFRESH_DISPLAY_MENU            2
#define _REFRESH_DISPLAY_OPTION_1        3
#define _REFRESH_DISPLAY_OPTION_2        4

typedef struct
{
    uint8_t id;
    uint8_t type;
    uint8_t nextStateId;
    uint8_t prevStateId;
    uint8_t clickStateId;
    uint8_t eepromRegister;
    uint16_t valueMin;
    uint16_t valueMax;
    ptrStateFunction callback;
} state_t;

const char *g_text[] = {
    "",                                     // _STATE_NO_MENU
    "Start/Stop",                           // _STATE_START_STOP
    "Puz On/Off",                           // _STATE_SNAIL
    "Temp",                                 // _STATE_TEMPERATURE
    "Potpala",                              // _STATE_STARTING
    "Stabiiliza",                           // _STATE_STABILISATION
    "Rad",                                  // _STATE_RUNNING
    "Gasenje",                              // _STATE_STOPPING
    "Plamen",                               // _STATE_FLAME_SENSOR
    "Datum i Vr",                           // _STATE_DATE_TIME
    "Servis",                               // _STATE_ADMINISTRATION
    "Izlaz",                                // _STATE_EXIT

    "Min",                                  // _STATE_TEMPERATURE_MIN
    "Max",                                  // _STATE_TEMPERATURE_MAX
    "Izlaz",                                // _STATE_TEMPERATURE_EX

    "Min",                                  // _STATE_TEMPERATURE_MIM_O
    "Max",                                  // _STATE_TEMPERATURE_MAX_O

    "Brzina V 1",                           // _STATE_START_FAN1_T
    "Vreme V 1",                            // _STATE_START_FAN1_S
    "Vreme D",                              // _STATE_START_DISPENSER
    "Vreme G",                              // _STATE_START_HEATER_T
    "Brzina V 2",                           // _STATE_START_FAN2_T
    "Vreme V 2",                            // _STATE_START_FAN2_S
    "Izlaz",                                // _STATE_START_EX

    "Brzina V 1",                           // _STATE_START_FAN1_T_O
    "Vreme V 1",                            // _STATE_START_FAN1_S_O
    "Vreme D",                              // _STATE_START_DISPENSER_O
    "Vreme G",                              // _STATE_START_HEATER_T_O
    "Brzina V 2",                           // _STATE_START_FAN2_T_O
    "Vreme V 2",                            // _STATE_START_FAN2_S_O

    "Dozator on",                           // _STATE_STAB_DISPENSER_ON
    "Dozator of",                           // _STATE_STAB_DISPENSER_OFF
    "Vreme V",                              // _STATE_STAB_FAN_S
    "Broj pon.",                            // _STATE_STAB_TOTAL_T
    "Izlaz",                                // _STATE_STAB_EX

    "Dozator on",                           // _STATE_STAB_DISPENSER_ON_O
    "Dozator of",                           // _STATE_STAB_DISPENSER_OFF_O
    "Vreme V",                              // _STATE_STAB_FAN_S_O
    "Ukupno vr",                            // _STATE_STAB_TOTAL_T_O

    "Dozator on",                           // _STATE_RUN_DISPENSER_ON
    "Dozator of",                           // _STATE_RUN_DISPENSER_OFF
    "Vreme V",                              // _STATE_RUN_FAN_S
    "Izlaz",                                // _STATE_RUN_EX

    "Dozator on",                           // _STATE_RUN_DISPENSER_ON_O
    "Dozator of",                           // _STATE_RUN_DISPENSER_OFF_O
    "Vreme V",                              // _STATE_RUN_FAN_S_O

    "Brzina V",                             // _STATE_STOP_FAN_T
    "Vreme V",                              // _STATE_STOP_FAN_S
    "Izlaz",                                // _STATE_STOP_EX

    "Brzina V",                             // _STATE_STOP_FAN_T_O
    "Vreme V",                              // _STATE_STOP_FAN_S_O

    "Pl min",                               // _STATE_FLAME_MIN
    "Pl min v",                             // _STATE_FLAME_MIN_T
    "Pl max",                               // _STATE_FLAME_MAX
    "Pl max v",                             // _STATE_FLAME_MAX_T
    "Izlaz",                                // _STATE_FLAME_EX

    "Pl min",                               // _STATE_FLAME_MIN_O
    "Pl min v",                             // _STATE_FLAME_MIN_T_O
    "Pl max",                               // _STATE_FLAME_MAX_O
    "Pl max v",                             // _STATE_FLAME_MAX_T_O

    "Krit temp",                            // _STATE_ADMIN_TEMP_CRT
    "Start temp",                           // _STATE_ADMIN_TEMP_START
    "Izlaz",                                // _STATE_ADMIN_EX

    "Krit temp",                            // _STATE_ADMIN_TEMP_CRT_O
    "Start temp",                           // _STATE_ADMIN_TEMP_START_O
};

static state_t g_states[] = {
//    Current state                 Type                Next state                  Prev state                      Click state                         Eeprom register                                 Value min   Value max   Callback
    { _STATE_NO_MENU,               _TYPE_SUBMENU,      _STATE_NO_MENU,             _STATE_NO_MENU,                 _STATE_START_STOP,                  0,                                              0,          0,          0},
    { _STATE_START_STOP,            _TYPE_ACTION,       _STATE_SNAIL,               _STATE_START_STOP,              _STATE_NO_MENU,                     0,                                              0,          0,          SM_startStop},
    { _STATE_SNAIL,                 _TYPE_ACTION,       _STATE_TEMPERATURE,         _STATE_START_STOP,              _STATE_NO_MENU,                     0,                                              0,          0,          SM_snailStartStop},
    { _STATE_TEMPERATURE,           _TYPE_SUBMENU,      _STATE_STARTING,            _STATE_SNAIL,                   _STATE_TEMPERATURE_MIN,             0,                                              0,          0,          0},
    { _STATE_STARTING,              _TYPE_SUBMENU,      _STATE_STABILISATION,       _STATE_TEMPERATURE,             _STATE_START_FAN1_T,                0,                                              0,          0,          0},
    { _STATE_STABILISATION,         _TYPE_SUBMENU,      _STATE_RUNNING,             _STATE_STARTING,                _STATE_STAB_DISPENSER_ON,           0,                                              0,          0,          0},
    { _STATE_RUNNING,               _TYPE_SUBMENU,      _STATE_STOPPING,            _STATE_STABILISATION,           _STATE_RUN_DISPENSER_ON,            0,                                              0,          0,          0},
    { _STATE_STOPPING,              _TYPE_SUBMENU,      _STATE_FLAME_SENSOR,        _STATE_RUNNING,                 _STATE_STOP_FAN_T,                  0,                                              0,          0,          0},
    { _STATE_FLAME_SENSOR,          _TYPE_SUBMENU,      _STATE_DATE_TIME,           _STATE_STOPPING,                _STATE_FLAME_MIN,                   0,                                              0,          0,          0},
    { _STATE_DATE_TIME,             _TYPE_SUBMENU,      _STATE_ADMINISTRATION,      _STATE_FLAME_SENSOR,            _STATE_DATE_TIME,                   0,                                              0,          0,          0},
    { _STATE_ADMINISTRATION,        _TYPE_SUBMENU,      _STATE_ADMINISTRATION,      _STATE_DATE_TIME,               _STATE_ADMIN_TEMP_CRT,              0,                                              0,          0,          0},
    { _STATE_EXIT,                  _TYPE_SUBMENU,      _STATE_EXIT,                _STATE_ADMINISTRATION,          _STATE_NO_MENU,                     0,                                              0,          0,          0},

    {_STATE_TEMPERATURE_MIN,        _TYPE_SUBMENU,      _STATE_TEMPERATURE_MAX,     _STATE_TEMPERATURE_MIN,         _STATE_TEMPERATURE_MIN_O,           0,                                              0,          0,          0},
    {_STATE_TEMPERATURE_MAX,        _TYPE_SUBMENU,      _STATE_TEMPERATURE_EX,      _STATE_TEMPERATURE_MIN,         _STATE_TEMPERATURE_MAX_O,           0,                                              0,          0,          0},
    {_STATE_TEMPERATURE_EX,         _TYPE_SUBMENU,      _STATE_TEMPERATURE_EX,      _STATE_TEMPERATURE_MAX,         _STATE_TEMPERATURE,                 0,                                              0,          0,          0},

    {_STATE_TEMPERATURE_MIN_O,      _TYPE_OPTION,       0,                          0,                              _STATE_TEMPERATURE_MIN,             EEPROM_ADDR_GLOBAL_TEMP_MIN,                    0,          99,         0},
    {_STATE_TEMPERATURE_MAX_O,      _TYPE_OPTION,       0,                          0,                              _STATE_TEMPERATURE_MAX,             EEPROM_ADDR_GLOBAL_TEMP_MAX,                    0,          99,         0},

    {_STATE_START_FAN1_T,           _TYPE_SUBMENU,      _STATE_START_FAN1_S,        _STATE_START_FAN1_T,            _STATE_START_FAN1_T_O,              0,                                              0,          0,          0},
    {_STATE_START_FAN1_S,           _TYPE_SUBMENU,      _STATE_START_DISPENSER,     _STATE_START_FAN1_T,            _STATE_START_FAN1_S_O,              0,                                              0,          0,          0},
    {_STATE_START_DISPENSER,        _TYPE_SUBMENU,      _STATE_START_HEATER_T,      _STATE_START_FAN1_S,            _STATE_START_DISPENSER_O,           0,                                              0,          0,          0},
    {_STATE_START_HEATER_T,         _TYPE_SUBMENU,      _STATE_START_FAN2_T,        _STATE_START_DISPENSER,         _STATE_START_HEATER_T_O,            0,                                              0,          0,          0},
    {_STATE_START_FAN2_T,           _TYPE_SUBMENU,      _STATE_START_FAN2_S,        _STATE_START_HEATER_T,          _STATE_START_FAN2_T_O,              0,                                              0,          0,          0},
    {_STATE_START_FAN2_S,           _TYPE_SUBMENU,      _STATE_START_EX,            _STATE_START_FAN2_T,            _STATE_START_FAN2_S_O,              0,                                              0,          0,          0},
    {_STATE_START_EX,               _TYPE_SUBMENU,      _STATE_START_EX,            _STATE_START_FAN2_S,            _STATE_STARTING,                    0,                                              0,          0,          0},

    {_STATE_START_FAN1_T_O,         _TYPE_OPTION,       0,                          0,                              _STATE_START_FAN1_T,                EEPROM_ADDR_STARTING_FAN_TIME,                  0,          300,        0},
    {_STATE_START_FAN1_S_O,         _TYPE_OPTION,       0,                          0,                              _STATE_START_FAN1_S,                EEPROM_ADDR_STARTING_FAN_SPEED,                 0,          100,        0},
    {_STATE_START_DISPENSER_O,      _TYPE_OPTION,       0,                          0,                              _STATE_START_DISPENSER,             EEPROM_ADDR_STARTING_DISPENSER_TIME,            0,          100,        0},
    {_STATE_START_HEATER_T_O,       _TYPE_OPTION,       0,                          0,                              _STATE_START_HEATER_T,              EEPROM_ADDR_STARTING_HEATER_TIME,               0,          999,        0},
    {_STATE_START_FAN2_T_O,         _TYPE_OPTION,       0,                          0,                              _STATE_START_FAN2_T,                EEPROM_ADDR_STARTING_FAN2_WAITING_TIME,         0,          300,        0},
    {_STATE_START_FAN2_S_O,         _TYPE_OPTION,       0,                          0,                              _STATE_START_FAN2_S,                EEPROM_ADDR_STARTING_FAN2_SPEED,                0,          100,        0},

    {_STATE_STAB_DISPENSER_ON,      _TYPE_SUBMENU,      _STATE_STAB_DISPENSER_OFF,  _STATE_STAB_DISPENSER_ON,       _STATE_STAB_DISPENSER_ON_O,         0,                                              0,          0,          0},
    {_STATE_STAB_DISPENSER_OFF,     _TYPE_SUBMENU,      _STATE_STAB_FAN_S,          _STATE_STAB_DISPENSER_ON,       _STATE_STAB_DISPENSER_OFF_O,        0,                                              0,          0,          0},
    {_STATE_STAB_FAN_S,             _TYPE_SUBMENU,      _STATE_STAB_TOTAL_T,        _STATE_STAB_DISPENSER_OFF,      _STATE_STAB_FAN_S_O,                0,                                              0,          0,          0},
    {_STATE_STAB_TOTAL_T,           _TYPE_SUBMENU,      _STATE_STAB_EX,             _STATE_STAB_FAN_S,              _STATE_STAB_TOTAL_T_O,              0,                                              0,          0,          0},
    {_STATE_STAB_EX,                _TYPE_SUBMENU,      _STATE_STAB_EX,             _STATE_STAB_TOTAL_T,            _STATE_STABILISATION,               0,                                              0,          0,          0},

    {_STATE_STAB_DISPENSER_ON_O,    _TYPE_OPTION,       0,                          0,                              _STATE_STAB_DISPENSER_ON,           EEPROM_ADDR_STABILISATION_DISPENSER_TIME_ON,    0,          99,         0},
    {_STATE_STAB_DISPENSER_OFF_O,   _TYPE_OPTION,       0,                          0,                              _STATE_STAB_DISPENSER_OFF,          EEPROM_ADDR_STABILISATION_DISPENSER_TIME_OFF,   0,          99,         0},
    {_STATE_STAB_FAN_S_O,           _TYPE_OPTION,       0,                          0,                              _STATE_STAB_FAN_S,                  EEPROM_ADDR_STABILISATION_FAN_SPEED,            0,          100,        0},
    {_STATE_STAB_TOTAL_T_O,         _TYPE_OPTION,       0,                          0,                              _STATE_STAB_TOTAL_T,                EEPROM_ADDR_STABILISATION_TOTAL_TIME,           0,          300,        0},

    {_STATE_RUN_DISPENSER_ON,       _TYPE_SUBMENU,      _STATE_RUN_DISPENSER_OFF,   _STATE_RUN_DISPENSER_ON,        _STATE_RUN_DISPENSER_ON_O,          0,                                              0,          0,          0},
    {_STATE_RUN_DISPENSER_OFF,      _TYPE_SUBMENU,      _STATE_RUN_FAN_S,           _STATE_RUN_DISPENSER_ON,        _STATE_RUN_DISPENSER_OFF_O,         0,                                              0,          0,          0},
    {_STATE_RUN_FAN_S,              _TYPE_SUBMENU,      _STATE_RUN_EX,              _STATE_RUN_DISPENSER_OFF,       _STATE_RUN_FAN_S_O,                 0,                                              0,          0,          0},
    {_STATE_RUN_EX,                 _TYPE_SUBMENU,      _STATE_RUN_EX,              _STATE_RUN_FAN_S,               _STATE_RUNNING,                     0,                                              0,          0,          0},

    {_STATE_RUN_DISPENSER_ON_O,     _TYPE_OPTION,       0,                          0,                              _STATE_RUN_DISPENSER_ON,            EEPROM_ADDR_RUNNING_DISPENSER_TIME_ON,          0,          99,         0},
    {_STATE_RUN_DISPENSER_OFF_O,    _TYPE_OPTION,       0,                          0,                              _STATE_RUN_DISPENSER_OFF,           EEPROM_ADDR_RUNNING_DISPENSER_TIME_OFF,         0,          99,         0},
    {_STATE_RUN_FAN_S_O,            _TYPE_OPTION,       0,                          0,                              _STATE_RUN_FAN_S,                   EEPROM_ADDR_RUNNING_FAN_SPEED,                  0,          100,        0},

    {_STATE_STOP_FAN_T,             _TYPE_SUBMENU,      _STATE_STOP_FAN_S,          _STATE_STOP_FAN_T,              _STATE_STOP_FAN_T_O,                0,                                              0,          0,          0},
    {_STATE_STOP_FAN_S,             _TYPE_SUBMENU,      _STATE_STOP_EX,             _STATE_STOP_FAN_T,              _STATE_STOP_FAN_S_O,                0,                                              0,          0,          0},
    {_STATE_STOP_EX,                _TYPE_SUBMENU,      _STATE_STOP_EX,             _STATE_STOP_FAN_S,              _STATE_STOPPING,                    0,                                              0,          0,          0},

    {_STATE_STOP_FAN_T_O,           _TYPE_OPTION,       0,                          0,                              _STATE_STOP_FAN_T,                  EEPROM_ADDR_STOPPING_FAN_TIME,                  0,          300,        0},
    {_STATE_STOP_FAN_S_O,           _TYPE_OPTION,       0,                          0,                              _STATE_STOP_FAN_S,                  EEPROM_ADDR_STOPPING_FAN_SPEED,                 0,          99,         0},

    {_STATE_FLAME_MIN,              _TYPE_SUBMENU,      _STATE_FLAME_MIN_T,         _STATE_FLAME_MIN,               _STATE_FLAME_MIN_O,                 0,                                              0,          0,          0},
    {_STATE_FLAME_MIN_T,            _TYPE_SUBMENU,      _STATE_FLAME_MAX,           _STATE_FLAME_MIN,               _STATE_FLAME_MIN_T_O,               0,                                              0,          0,          0},
    {_STATE_FLAME_MAX,              _TYPE_SUBMENU,      _STATE_FLAME_MAX_T,         _STATE_FLAME_MIN_T,             _STATE_FLAME_MAX_O,                 0,                                              0,          0,          0},
    {_STATE_FLAME_MAX_T,            _TYPE_SUBMENU,      _STATE_FLAME_EX,            _STATE_FLAME_MAX,               _STATE_FLAME_MAX_T_O,               0,                                              0,          0,          0},
    {_STATE_FLAME_EX,               _TYPE_SUBMENU,      _STATE_FLAME_EX,            _STATE_FLAME_MAX_T,             _STATE_FLAME_SENSOR,                0,                                              0,          0,          0},

    {_STATE_FLAME_MIN_O,            _TYPE_OPTION,       0,                          0,                              _STATE_FLAME_MIN,                   EEPROM_ADDR_STARTING_FLAME_MIN,                 0,          100,        0},
    {_STATE_FLAME_MIN_T_O,          _TYPE_OPTION,       0,                          0,                              _STATE_FLAME_MIN_T,                 EEPROM_ADDR_STARTING_FLAME_TIME,                0,          99,         0},
    {_STATE_FLAME_MAX_O,            _TYPE_OPTION,       0,                          0,                              _STATE_FLAME_MAX,                   EEPROM_ADDR_STOPPING_FLAME_MAX,                 0,          100,        0},
    {_STATE_FLAME_MAX_T_O,          _TYPE_OPTION,       0,                          0,                              _STATE_FLAME_MAX_T,                 EEPROM_ADDR_STOPPING_FLAME_TIME,                0,          99,         0},

    {_STATE_ADMIN_TEMP_CRT,         _TYPE_SUBMENU,      _STATE_ADMIN_TEMP_START,    _STATE_ADMIN_TEMP_CRT,          _STATE_ADMIN_TEMP_CRT_O,            0,                                              0,          0,          0},
    {_STATE_ADMIN_TEMP_START,       _TYPE_SUBMENU,      _STATE_ADMIN_EX,            _STATE_ADMIN_TEMP_CRT,          _STATE_ADMIN_TEMP_START_O,          0,                                              0,          0,          0},
    {_STATE_ADMIN_EX,               _TYPE_SUBMENU,      _STATE_ADMIN_EX,            _STATE_ADMIN_TEMP_START,        _STATE_ADMINISTRATION,              0,                                              0,          0,          0},

    {_STATE_ADMIN_TEMP_CRT_O,       _TYPE_OPTION,       0,                          0,                              _STATE_ADMIN_TEMP_CRT,              EEPROM_ADDR_GLOBAL_TEMP_CRITICAL,               0,          99,        0},
    {_STATE_ADMIN_TEMP_START_O,     _TYPE_OPTION,       0,                          0,                              _STATE_ADMIN_TEMP_START,            EEPROM_ADDR_GLOBAL_TEMP_START,                  0,          99,         0},
};

static uint8_t g_currentState;

static uint8_t g_btnLastState;
static uint8_t g_encALastState;
static uint8_t g_encBLastState;

static uint16_t g_optionValue;

static volatile uint8_t g_refreshDisplay;

static void _handleClick(void);
static void _handleNext(void);
static void _handlePrev(void);

static void _showMenuItem(const char *str1, const char *str2, const char *str3);
static void _showMenuOption(uint16_t value);

void MENU_init(void)
{
    cli();
    DDRB &= ~(1 << PB4) | ~(1 << PB5) | ~(1 << PB6);
    PCICR |= (1 << PCIE0);
    //PCMSK0 |= (1 << PCINT4) | (1 << PCINT5) | (1 << PCINT6);
    PCMSK0 |= (1 << PCINT4) | (1 << PCINT6);
    sei();

    g_currentState = _STATE_START_STOP;
    g_encALastState = (PINB >> PB4) & 0x01;
    g_encBLastState = (PINB >> PB5) & 0x01;
    g_btnLastState = (PINB >> PB6) & 0x01;

    DISPLAY_gotoXY(0, 0);
    DISPLAY_showString("        |");
    DISPLAY_gotoXY(0, 1);
    DISPLAY_showString("        |");
    DISPLAY_gotoXY(0, 2);
    DISPLAY_showString("        |");
    DISPLAY_gotoXY(0, 3);
    DISPLAY_showString("16.10.19|");

    DISPLAY_gotoXY(12, 0);
    DISPLAY_showString("MENI:");
    DISPLAY_gotoXY(9, 2);
    DISPLAY_showString(">");

    //g_refreshDisplay = _REFRESH_DISPLAY_INIT;
    _showMenuItem(g_text[g_currentState], g_text[g_states[g_currentState].nextStateId], 0);
}

void MENU_refreshSensorValue(uint16_t temperature, uint16_t flame)
{
    uint16_t flameMin = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);

    DISPLAY_gotoXY(0, 0);
    DISPLAY_showString("        ");
    DISPLAY_gotoXY(0, 0);
    DISPLAY_showString("S:");
    DISPLAY_showInteger(flameMin);
    DISPLAY_showString("/");
    DISPLAY_showInteger(flame);
    DISPLAY_gotoXY(0, 1);
    DISPLAY_showString("        ");
    DISPLAY_gotoXY(0, 1);
    DISPLAY_showString("T:");
    DISPLAY_showInteger(temperature);
    DISPLAY_showString("C");
}

void MENU_refreshError(const char *str1, const char *str2)
{
    DISPLAY_gotoXY(0, 2);
    DISPLAY_showString("        ");
    DISPLAY_gotoXY(0, 2);
    DISPLAY_showString(str1);

    DISPLAY_gotoXY(0, 3);
    DISPLAY_showString("        ");
    DISPLAY_gotoXY(0, 3);
    DISPLAY_showString(str2);
}

void MENU_refresh(void)
{
    switch (g_refreshDisplay)
    {
        case _REFRESH_DISPLAY_INIT:
            DISPLAY_init();
            DISPLAY_gotoXY(0, 0);
            DISPLAY_showString("        |");
            DISPLAY_gotoXY(0, 1);
            DISPLAY_showString("        |");
            DISPLAY_gotoXY(0, 2);
            DISPLAY_showString("        |");
            DISPLAY_gotoXY(0, 3);
            DISPLAY_showString("16.10.19|");

            DISPLAY_gotoXY(12, 0);
            DISPLAY_showString("MENI:");
            DISPLAY_gotoXY(9, 2);
            DISPLAY_showString(">");

            _showMenuItem(g_text[g_currentState], g_text[g_states[g_currentState].nextStateId], 0);
            break;
        case _REFRESH_DISPLAY_MENU:
            do
            {
                const char *str1 = g_text[g_currentState];
                const char *str2 = 0;
                const char *str3 = 0;

                if (g_currentState != g_states[g_currentState].nextStateId && g_states[g_currentState].nextStateId != 0)
                {
                    str2 = g_text[g_states[g_currentState].nextStateId];
                }
            
                if (g_currentState != g_states[g_currentState].prevStateId && g_states[g_currentState].prevStateId != 0)
                {
                    str3 = g_text[g_states[g_currentState].prevStateId];
                }

                _showMenuItem(str1, str2, str3);
            } while (0);
            break;
        case _REFRESH_DISPLAY_OPTION_1:
            _showMenuItem(g_text[g_currentState], 0, 0);
            g_optionValue = EEPROM_readWord(g_states[g_currentState].eepromRegister);
            if (g_optionValue < g_states[g_currentState].valueMin)
            {
                g_optionValue = g_states[g_currentState].valueMin;
            }
            else if (g_optionValue > g_states[g_currentState].valueMax)
            {
                g_optionValue = g_states[g_currentState].valueMax;
            }
            _showMenuOption(g_optionValue);
            break;
        case _REFRESH_DISPLAY_OPTION_2:
            _showMenuOption(g_optionValue);
            break;
    }

    g_refreshDisplay = _REFRESH_DISPLAY_NO;
}

static void _handleClick(void)
{
    switch (g_states[g_currentState].type)
    {
    case _TYPE_SUBMENU:
        g_currentState = g_states[g_currentState].clickStateId;
        if (g_states[g_currentState].type == _TYPE_OPTION)
        {
            _showMenuItem(g_text[g_currentState], 0, 0);
            g_optionValue = EEPROM_readWord(g_states[g_currentState].eepromRegister);
            if (g_optionValue < g_states[g_currentState].valueMin)
            {
                g_optionValue = g_states[g_currentState].valueMin;
            }
            else if (g_optionValue > g_states[g_currentState].valueMax)
            {
                g_optionValue = g_states[g_currentState].valueMax;
            }
            _showMenuOption(g_optionValue);

            //g_refreshDisplay = _REFRESH_DISPLAY_OPTION_1;
        }
        else
        {
            const char *str1 = g_text[g_currentState];
            const char *str2 = 0;
            const char *str3 = 0;

            if (g_currentState != g_states[g_currentState].nextStateId && g_states[g_currentState].nextStateId != 0)
            {
                str2 = g_text[g_states[g_currentState].nextStateId];
            }
            
            if (g_currentState != g_states[g_currentState].prevStateId && g_states[g_currentState].prevStateId != 0)
            {
                str3 = g_text[g_states[g_currentState].prevStateId];
            }
            _showMenuItem(str1, str2, str3);

            //g_refreshDisplay = _REFRESH_DISPLAY_OPTION_2;
        }
        break;
    case _TYPE_OPTION:
        EEPROM_writeWord(g_states[g_currentState].eepromRegister, g_optionValue);

        g_currentState = g_states[g_currentState].clickStateId;

        const char *str1 = g_text[g_currentState];
        const char *str2 = 0;
        const char *str3 = 0;

        if (g_currentState != g_states[g_currentState].nextStateId && g_states[g_currentState].nextStateId != 0)
        {
            str2 = g_text[g_states[g_currentState].nextStateId];
        }
            
        if (g_currentState != g_states[g_currentState].prevStateId && g_states[g_currentState].prevStateId != 0)
        {
            str3 = g_text[g_states[g_currentState].prevStateId];
        }
        _showMenuItem(str1, str2, str3);

        //g_refreshDisplay = _REFRESH_DISPLAY_MENU;
        break;
    case _TYPE_ACTION:
        if (g_states[g_currentState].callback)
        {
            g_states[g_currentState].callback();
        }
        break;
    }
}

static void _handleNext(void)
{
    if (g_states[g_currentState].type != _TYPE_OPTION)
    {
        if (g_currentState != g_states[g_currentState].nextStateId)
        {
            g_currentState = g_states[g_currentState].nextStateId;

            const char *str1 = g_text[g_currentState];
            const char *str2 = 0;
            const char *str3 = 0;

            if (g_currentState != g_states[g_currentState].nextStateId && g_states[g_currentState].nextStateId != 0)
            {
                str2 = g_text[g_states[g_currentState].nextStateId];
            }
            
            if (g_currentState != g_states[g_currentState].prevStateId && g_states[g_currentState].prevStateId != 0)
            {
                str3 = g_text[g_states[g_currentState].prevStateId];
            }
            _showMenuItem(str1, str2, str3);

            //g_refreshDisplay = _REFRESH_DISPLAY_MENU;
        }
    }
    else
    {
        if (g_optionValue < g_states[g_currentState].valueMax)
        {
            g_optionValue++;
            _showMenuOption(g_optionValue);
            //g_refreshDisplay = _REFRESH_DISPLAY_OPTION_2;
        }
    }
}

static void _handlePrev(void)
{
    if (g_states[g_currentState].type != _TYPE_OPTION)
    {
        if (g_currentState != g_states[g_currentState].prevStateId)
        {
            g_currentState = g_states[g_currentState].prevStateId;

            const char *str1 = g_text[g_currentState];
            const char *str2 = 0;
            const char *str3 = 0;

            if (g_currentState != g_states[g_currentState].nextStateId && g_states[g_currentState].nextStateId != 0)
            {
                str2 = g_text[g_states[g_currentState].nextStateId];
            }
            
            if (g_currentState != g_states[g_currentState].prevStateId && g_states[g_currentState].prevStateId != 0)
            {
                str3 = g_text[g_states[g_currentState].prevStateId];
            }
            _showMenuItem(str1, str2, str3);

            //g_refreshDisplay = _REFRESH_DISPLAY_MENU;
        }
    }
    else
    {
        if (g_optionValue > g_states[g_currentState].valueMin)
        {
            g_optionValue--;
            _showMenuOption(g_optionValue);
            //g_refreshDisplay = _REFRESH_DISPLAY_OPTION_2;
        }
    }
}

static void _showMenuItem(const char *str1, const char *str2, const char *str3)
{
    DISPLAY_gotoXY(10, 1);
    DISPLAY_showString("          ");
    if (str3)
    {
        DISPLAY_gotoXY(10, 1);
        DISPLAY_showString(str3);
    }
    DISPLAY_gotoXY(10, 2);
    DISPLAY_showString("          ");
    DISPLAY_gotoXY(10, 2);
    DISPLAY_showString(str1);
    DISPLAY_gotoXY(10, 3);
    DISPLAY_showString("          ");
    if (str2)
    {
        DISPLAY_gotoXY(10, 3);
        DISPLAY_showString(str2);
    }
}

static void _showMenuOption(uint16_t value)
{
    DISPLAY_gotoXY(10, 3);
    DISPLAY_showString("          ");
    DISPLAY_gotoXY(10, 3);
    DISPLAY_showInteger(value);
}

ISR(PCINT0_vect)
{
    //uint8_t intFlags = PORTC.INTFLAGS;
    //PORTC.INTFLAGS = intFlags;

    uint8_t encACurrentState = (PINB >> PB4) & 0x01;
    uint8_t encBCurrentState = (PINB >> PB5) & 0x01;
    uint8_t btnCurrentState = (PINB >> PB6) & 0x01;

    if (encACurrentState == g_encALastState && encBCurrentState == g_encBLastState && btnCurrentState == g_btnLastState)
    {
        return;
    }

    int8_t side = 0;

    if (btnCurrentState && !g_btnLastState)
    {
        _handleClick();
    }

    if (encACurrentState != g_encALastState && encACurrentState == 0)
    {
        if (encACurrentState != encBCurrentState)
        {
            side = -1;
        }
        else
        {
            side = 1;
        }
    }

    if (side > 0)
    {
        _handleNext();
    }
    else if (side < 0)
    {
        _handlePrev();
    }

    g_encALastState = encACurrentState;
    g_encBLastState = encBCurrentState;
    g_btnLastState = btnCurrentState;
}
