/*
 * File name:       statemachine.c
 * Description:     Main logic
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-31
 * 
 */

#include "system.h"
#include "statemachine.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "driver/uart.h"
#include "driver/eeprom.h"
#include "driver/max6675.h"
#include "driver/ds18b20.h"
#include "driver/adc.h"
#include "driver/pwm.h"
#include "driver/pcf8574.h"
#include "driver/mcp7940.h"
#include "gpio.h"
#include "display.h"
#include "debug.h"

#define _STATE_STOPPED                  0
#define _STATE_STARTING                 1
#define _STATE_STABILISATION            2
#define _STATE_RUNNING                  3
#define _STATE_STOPPING                 4
#define _STATE_WAITING                  5
#define _STATE_SNAIL                    6
#define _STATE_CRITICAL                 7

#define _RESULT_SUCCESS                 0
#define _RESULT_REPEATE                 1
#define _RESULT_ERROR                   2
#define _RESULT_CRITICAL                3

#define _ERROR_NO                               0
#define _ERROR_TEMPERATURE_CRITICAL             1
#define _ERROR_TEMPERATURE_DISCONNECTED         2
#define _ERROR_FLAME_DISCONNECTED               3

/*
 * For sending to display
 */

#define _STATE_DEVICE_UNKNOWN               0
#define _STATE_DEVICE_STOPPED               1
#define _STATE_DEVICE_VENTILATION           3
#define _STATE_DEVICE_STARTING              4
#define _STATE_DEVICE_STABILISATION         5
#define _STATE_DEVICE_RUNNING               6
#define _STATE_DEVICE_STOPPING              7
#define _STATE_DEVICE_WAITING               8
#define _STATE_DEVICE_SNAIL                 9
#define _STATE_DEVICE_CRITICAL             10

typedef uint8_t (*ptrStateFunction)(void);

typedef struct
{
    uint8_t id;
    uint8_t successStateId;
    uint8_t repeateStateId;
    uint8_t errorStateId;
    uint8_t criticalStateId;
    ptrStateFunction callback;
} state_t;

static void _SM_checkSensors(void);
static void _SM_checkError(void);

static uint8_t _SM_stateStopped(void);
static uint8_t _SM_stateStarting(void);
static uint8_t _SM_stateStabilisation(void);
static uint8_t _SM_stateRunning(void);
static uint8_t _SM_stateStopping(void);
static uint8_t _SM_stateWaiting(void);
static uint8_t _SM_stateSnail(void);
static uint8_t _SM_stateCritical(void);

static state_t g_states[] = {
//    Current state         Success state           Repeate state           Error state             Critical state          Callback
    { _STATE_STOPPED,       _STATE_WAITING,         _STATE_STOPPED,         _STATE_SNAIL,           _STATE_CRITICAL,        _SM_stateStopped },
    { _STATE_STARTING,      _STATE_STABILISATION,   _STATE_STARTING,        _STATE_STOPPING,        _STATE_CRITICAL,        _SM_stateStarting },
    { _STATE_STABILISATION, _STATE_RUNNING,         _STATE_STABILISATION,   _STATE_STOPPING,        _STATE_CRITICAL,        _SM_stateStabilisation },
    { _STATE_RUNNING,       _STATE_STOPPING,        _STATE_RUNNING,         _STATE_STOPPING,        _STATE_CRITICAL,        _SM_stateRunning },
    { _STATE_STOPPING,      _STATE_WAITING,         _STATE_STOPPING,        _STATE_STOPPED,         _STATE_CRITICAL,        _SM_stateStopping },
    { _STATE_WAITING,       _STATE_STARTING,        _STATE_WAITING,         _STATE_STOPPED,         _STATE_CRITICAL,        _SM_stateWaiting },
    { _STATE_SNAIL,         _STATE_STOPPED,         _STATE_SNAIL,           _STATE_STOPPED,         _STATE_CRITICAL,        _SM_stateSnail },
    { _STATE_CRITICAL,      _STATE_STOPPED,         _STATE_CRITICAL,        _STATE_STOPPED,         _STATE_CRITICAL,        _SM_stateCritical },
};

static uint8_t g_currentState;

static double g_temperature = -1.0;
static double g_flame = 0;

volatile static uint8_t g_snailRunning = 0;

static uint8_t g_lastError;
static uint8_t g_error;

static uint8_t g_seconds;
static uint8_t g_minutes = 0xFF;            // To refresh first time when read
static uint8_t g_hours;
static uint8_t g_days;
static uint8_t g_months;
static uint16_t g_years;

static void _uartCallback(uint8_t *buffer, uint8_t size);

static void _sendState(uint8_t state);

void SM_init(void)
{
    cli();

    g_currentState = _STATE_STOPPED;
    g_error = _ERROR_NO;
    g_lastError = g_error;

    /*
    DDRB |= (1 << PB5);
    while (1)
    {
        PORTB ^= (1 << PB5);
        _delay_ms(1000);
    }
    */

    DEBUG_init();

    //EEPROM_writeWord(EEPROM_ADDR_STARTING_FLAME_MIN, 0);

    //while (1);

    UART_init();
    UART_setReaceiveCallback(_uartCallback);

    //pcf8574_init();                                                         // PCF8574 init

    //GPIO_init();                                                            // Initialize GPIO (digital input/output pins)

    PWM0_init();                                                            // Initialize PWM0 (DC Motor)
    PWM1_init();                                                            // Initialize PWM1 (AC Motor -> FAN)
    PWM2_init();                                                            // Initialize PWM2 ()

    ADC_init();                                                             // Initialize ADC foto resistor

    //MCP7940_init();                                                         // Initialize MCP7940 RTC

    sei();                                                                  // Enable interrupts

    /*
    while (1)
    {
        GPIO_relayOn(GPIO_RELAY_HEATER);
        _delay_ms(1000);
        GPIO_relayOff(GPIO_RELAY_HEATER);
        _delay_ms(1000);
        //DEBUG_printf("NESTO\n");
    }
    */

    /*
    while (1)
    {
        if (ADC_connected(0b111))
        {
            uint16_t tmp = ADC_read(0b111);
            DEBUG_printf("ADC: %d\n", tmp);
        }
        else
        {
            DEBUG_printf("disconnected\n");
        }
    }
    */

    /*
    while (1)
    {
        PWM0_setDutyCycle(0);
        _delay_ms(1000);
        PWM0_setDutyCycle(125);
        _delay_ms(1000);
        PWM0_setDutyCycle(255);
        _delay_ms(1000);
    }
    */

    /*
    while (1)
    {
        PWM1_setFrequency(0);
        _delay_ms(1000);
        PWM1_setFrequency(50);
        _delay_ms(1000);
        PWM1_setFrequency(100);
        _delay_ms(1000);
    }
    */

    /*
    while (1)
    {
        uint16_t temp = (uint16_t)ds18b20_gettemp();
        DEBUG_printf("TEMP: %d\n", temp);
        _delay_ms(1000);
    }
    */

    //DISPLAY_init();                                                         // Initialize display

    //MENU_init();                                                            // Initialize menu

    g_minutes = 0xFF;
    _SM_checkSensors();                                                     // Check sensors for init time

    _delay_ms(1000);                                                        // Just in case :D

    if (EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
    {
        //uint16_t minTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MIN);
        uint16_t maxTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MAX);
        //uint16_t criticalTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);
        //uint16_t startTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);
        uint16_t flameMin = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
        //uint16_t flameMax = EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_MAX);

        double currentTemp = ds18b20_gettemp();
        uint16_t flame = ADC_read(0b111);
        flame = (uint16_t)SYSTEM_MAP(flame, 150.0, 1023.0, 0.0, 100.0);

        if (currentTemp > flameMin)
        {
            if (currentTemp > maxTemp)
            {
                g_currentState = _STATE_STOPPING;
            }
            else
            {
                g_currentState = _STATE_STABILISATION;
            }
        }
        else
        {
            g_currentState = _STATE_WAITING;
        }

        g_currentState = _STATE_WAITING;
    }
}

void SM_exec(void)
{
    uint8_t result;
    //uint8_t lastState;

    while (1)
    {
        result = g_states[g_currentState].callback();
        //lastState = g_currentState;

        switch (result)
        {
            case _RESULT_SUCCESS:
                g_currentState = g_states[g_currentState].successStateId;
                break;
            case _RESULT_REPEATE:
                g_currentState = g_states[g_currentState].repeateStateId;
                break;
            case _RESULT_ERROR:
                g_currentState = g_states[g_currentState].errorStateId;
                break;
            case _RESULT_CRITICAL:
                g_currentState = g_states[g_currentState].criticalStateId;
                break;
        }

        /*
        if (lastState != g_currentState)
        {
            g_stateInitialized = 0;
            g_stateStartTime = TIMER0_millis();
        }
        */
    }
}

void SM_start(void)
{
    EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 1);
    g_error = _ERROR_NO;
}

void SM_stop(void)
{
    EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 0);
    g_error = _ERROR_NO;
}

void SM_snailStart(void)
{
    //EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 1);
    g_snailRunning = 1;
}

void SM_snailStop(void)
{
    //EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 0);
    g_snailRunning = 0;
}

void SM_startStop(void)
{
    if (g_currentState == _STATE_STOPPED)
    {
        EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 1);
    }
    else
    {
        EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 0);
    }
}

void SM_snailStartStop(void)
{
    if (g_currentState == _STATE_STOPPED || g_currentState == _STATE_SNAIL)
    {
        if (g_snailRunning)
        {
            g_snailRunning = 0;
        }
        else
        {
            g_snailRunning = 1;
        }
    }
}

static void _SM_checkSensors(void)
{
    uint8_t bufferDisplayPause[] = { UART_ESC, UART_STX, 'p', 'p', UART_ESC, UART_ETX };

    while (UART_recevingInProgress())
    {
        _delay_ms(1);
    }
    UART_writeBuffer(bufferDisplayPause, 6);

    uint16_t temperatureCritical = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);
    uint16_t temperatureStart = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_START);
    uint16_t flameStart = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
    uint16_t flame = 0;
    double temperature = ds18b20_gettemp();
    //= ADC_read(0b111);

    if (g_error != _ERROR_TEMPERATURE_CRITICAL)
    {
        g_error = _ERROR_NO;
    }

    uint8_t i = 0;
    uint8_t tmpError = _ERROR_NO;
    while (((uint16_t)temperature) == 0)
    {
        temperature = ds18b20_gettemp();
        if (i > 3)
        {
            tmpError = _ERROR_TEMPERATURE_DISCONNECTED;
            break;
        }
        i++;
    }

    if (g_error == _ERROR_TEMPERATURE_CRITICAL)
    {
        if (tmpError != _ERROR_TEMPERATURE_DISCONNECTED)
        {
            if (temperature < temperatureStart)
            {
                //g_error = _ERROR_NO;
            }
        }
    }
    else
    {
        g_error = _ERROR_NO;
    }

    if (tmpError == _ERROR_TEMPERATURE_DISCONNECTED)
    {
        g_error = _ERROR_TEMPERATURE_DISCONNECTED;
    }

    if (temperature > temperatureCritical)
    {
        if (g_error != _ERROR_TEMPERATURE_CRITICAL)
        {
            g_error = _ERROR_TEMPERATURE_CRITICAL;
        }
    }

    //flame = SYSTEM_MAP(flame, 1020, 100, 0, 100);
    //flame = 100 - SYSTEM_MAP(flame, 100, 1023, 0, 100);

    if (ADC_connected(0b111))
    {
        flame = ADC_read(0b111);
        //flame = (uint16_t)SYSTEM_MAP(flame, 1023.0, 0.0, 0.0, 100.0);
        if (flame < 150)
        {
            flame = 0;
        }
        else
        {
            flame = (uint16_t)SYSTEM_MAP(flame, 150.0, 1023.0, 0.0, 100.0);
        }
    }
    else
    {
        if (g_error == _ERROR_NO)
        {
            g_error = _ERROR_FLAME_DISCONNECTED;
        }
    }

    /*
     * TODO: Only for testing - remove
     */
    //g_error = _ERROR_NO;
    //g_flame = flame;            

    if (g_flame != flame || g_temperature != temperature || g_error != _ERROR_NO)
    {
        /*
        UART_writeString("T: ");
        UART_writeIntegerString(temperature);
        UART_writeString("\n");
        UART_writeString("F: ");
        UART_writeIntegerString(flame);
        UART_writeString("\n");
        */
        //cli();
        //MENU_refreshSensorValue((uint16_t)temperature, (uint16_t)flame);
        uint8_t buffer[] = { UART_ESC, UART_STX, 's', 's', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UART_ESC, UART_ETX };
        buffer[4] = ((uint16_t)temperature >> 8) & 0xFF;
        buffer[5] = (uint16_t)temperature & 0xFF;
        buffer[6] = ((uint16_t)flame >> 8) & 0xFF;
        buffer[7] = (uint16_t)flame & 0xFF;
        buffer[8] = ((uint16_t)flameStart >> 8) & 0xFF;
        buffer[9] = (uint16_t)flameStart & 0xFF;
        UART_writeBuffer(buffer, 12);
        //sei();
    }

    uint8_t ss;
    uint8_t mm;
    uint8_t hh;
    uint8_t d;
    uint8_t m;
    uint16_t y;
    //cli();
    MCP7940_now(&ss, &mm, &hh, &d, &m, &y);
    if (mm != g_minutes || hh != g_hours || d != g_days || m != g_months || y != g_years)
    {
        g_minutes = mm;
        g_hours = hh;
        g_days = d;
        g_months = m;
        g_years = y;

        uint8_t buffer[] = { UART_ESC, UART_STX, 's', 't', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UART_ESC, UART_ETX };
        buffer[4] = g_seconds;
        buffer[5] = g_minutes;
        buffer[6] = g_hours;
        buffer[7] = g_days;
        buffer[8] = g_months;
        buffer[9] = (g_years >> 8) & 0xFF;
        buffer[10] = g_years & 0xFF;
        UART_writeBuffer(buffer, 13);
    }
    //sei();

    g_flame = flame;
    g_temperature = temperature;

    _SM_checkError();

    bufferDisplayPause[3] = 'r';
    UART_writeBuffer(bufferDisplayPause, 6);

    _delay_ms(100);                             // Workaround for UART to work
}

static void _SM_checkError(void)
{
    if (g_error != g_lastError)
    {
        //cli();
        uint8_t buffer[] = { UART_ESC, UART_STX, 'e', 'e', 0x00, UART_ESC, UART_ETX };
        if (g_error == _ERROR_NO)
        {
            buffer[3] = 'n';
        }
        else
        {
            buffer[4] = g_error;
        }
        UART_writeBuffer(buffer, 7);
        //sei();

        /*
        switch (g_error)
        {
            case _ERROR_NO:
                //MENU_refreshError("", "");
                buffer[3] = 'n';
                UART_writeBuffer(buffer, 7);
                break;
            case _ERROR_TEMPERATURE_CRITICAL:
                //MENU_refreshError("G:RAD", "KRIT TEM");
                buffer[4] = _;
                UART_writeBuffer(buffer, 7);
                break;
            case _ERROR_TEMPERATURE_DISCONNECTED:
                //MENU_refreshError("G:SENZOR", "TEMP");
                buffer[4] = ' ';
                UART_writeBuffer(buffer, 7);
                break;
            case _ERROR_FLAME_DISCONNECTED:
                //MENU_refreshError("G:SENZOR", "OKO");
                buffer[4] = ' ';
                UART_writeBuffer(buffer, 7);
                break;
        }
        sei();
        */
    }
    g_lastError = g_error;
}

static uint8_t _SM_stateStopped(void)
{
    _SM_checkSensors();

    TIME_reset();

    _sendState(_STATE_DEVICE_STOPPED);

    if (g_error == _ERROR_TEMPERATURE_CRITICAL)
    {
        uint16_t criticalTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);
        uint16_t startTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_START);
        uint16_t diffTemp = criticalTemp - startTemp;

        if (g_temperature >= diffTemp)
        {
            return _RESULT_CRITICAL;
        }
    }

    if (g_snailRunning)
    {
        PWM0_setDutyCycle(255);
        return _RESULT_ERROR;
    }

    if (EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
    {
        return _RESULT_SUCCESS;
    }

    return _RESULT_REPEATE;
}

static uint8_t _SM_stateStarting(void)
{
    uint8_t fan1Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_SPEED);
    uint32_t fan1Time = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_TIME) * 1000;
    uint32_t dispenserTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_DISPENSER_TIME) * 1000;
    uint32_t heaterTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_HEATER_TIME) * 1000;
    uint8_t fan2Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_SPEED);
    uint32_t fan2WaitingTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_WAITING_TIME) * 1000;
    uint16_t flameMin = EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_MAX);
    uint32_t flameTime = (uint16_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_TIME) * 1000;

    uint8_t fanStarted = 0;
    uint32_t flameStartTime = 0;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    _sendState(_STATE_DEVICE_VENTILATION);

    PWM1_setFrequency(fan1Speed);
    while (currentTime - startTime < fan1Time)
    {
        fan1Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_SPEED);
        fan1Time = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_TIME) * 1000;
        dispenserTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_DISPENSER_TIME) * 1000;
        heaterTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_HEATER_TIME) * 1000;
        fan2Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_SPEED);
        fan2WaitingTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_WAITING_TIME) * 1000;
        flameMin = EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_MAX);
        flameTime = (uint16_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_TIME) * 1000;

        PWM1_setFrequency(fan1Speed);

        //UART_writeString("STARTING: Fan 01\n");
        _SM_checkSensors();
        //UART_writeString("STARTING: Fan 02\n");
        //_delay_ms(10);
        //UART_writeString("STARTING: Fan 03\n");
        //UART_writeString("STARTING: Fan 04\n");

        currentTime = TIME_milis();
        
        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            return _RESULT_CRITICAL;
        }

        if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
        {
            return _RESULT_ERROR;
        }
    }
    PWM1_setFrequency(0);

    _sendState(_STATE_DEVICE_STARTING);

    PWM0_setDutyCycle(255);
    startTime = TIME_milis();
    currentTime = TIME_milis();
    while (currentTime - startTime < dispenserTime)
    {
        fan1Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_SPEED);
        fan1Time = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_TIME) * 1000;
        dispenserTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_DISPENSER_TIME) * 1000;
        heaterTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_HEATER_TIME) * 1000;
        fan2Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_SPEED);
        fan2WaitingTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_WAITING_TIME) * 1000;
        flameMin = EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_MAX);
        flameTime = (uint16_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_TIME) * 1000;

        _SM_checkSensors();

        currentTime = TIME_milis();

        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            return _RESULT_CRITICAL;
        }

        if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
        {
            return _RESULT_ERROR;
        }
    }
    PWM0_setDutyCycle(0);

    GPIO_relayOn(GPIO_RELAY_HEATER);
    startTime = TIME_milis();
    currentTime = TIME_milis();
    while (currentTime - startTime < heaterTime)
    {
        fan1Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_SPEED);
        fan1Time = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_TIME) * 1000;
        dispenserTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_DISPENSER_TIME) * 1000;
        heaterTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_HEATER_TIME) * 1000;
        fan2Speed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_SPEED);
        fan2WaitingTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_WAITING_TIME) * 1000;
        flameMin = EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_MAX);
        flameTime = (uint16_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_TIME) * 1000;

        _SM_checkSensors();

        currentTime = TIME_milis();

        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            return _RESULT_CRITICAL;
        }

        if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
        {
            return _RESULT_ERROR;
        }

        if (currentTime - startTime >= fan2WaitingTime && !fanStarted)
        {
            PWM1_setFrequency(fan2Speed);
            fanStarted = 1;
        }
        else if (fanStarted)
        {
            PWM1_setFrequency(fan2Speed);
        }

        if (g_flame > flameMin)
        {
            if (!flameStartTime)
            {
                flameStartTime = currentTime;
            }
        }
        else
        {
            flameStartTime = 0;
        }

        if (flameStartTime && currentTime - flameStartTime > flameTime)
        {
            PWM1_setFrequency(0);
            GPIO_relayOff(GPIO_RELAY_HEATER);
            return _RESULT_SUCCESS;
        }
    }
    PWM1_setFrequency(0);
    GPIO_relayOff(GPIO_RELAY_HEATER);

    return _RESULT_ERROR;
}

static uint8_t _SM_stateStabilisation(void)
{
    uint16_t stabilisationTime = EEPROM_readWord(EEPROM_ADDR_STABILISATION_TOTAL_TIME);
    uint8_t fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_FAN_SPEED);
    uint32_t timeDispenserOn = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_ON) * 1000;
    uint32_t timeDispenserOff = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_OFF) * 1000;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    if (stabilisationTime == 0)
    {
        return _RESULT_SUCCESS;
    }

    _sendState(_STATE_DEVICE_STABILISATION);

    PWM1_setFrequency(fanSpeed);

    for (uint16_t i = 0; i < stabilisationTime; i++)
    {
        stabilisationTime = EEPROM_readWord(EEPROM_ADDR_STABILISATION_TOTAL_TIME);
        fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_FAN_SPEED);
        timeDispenserOn = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_ON) * 1000;
        timeDispenserOff = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_OFF) * 1000;

        PWM0_setDutyCycle(255);
        startTime = TIME_milis();
        currentTime = TIME_milis();
        while (currentTime - startTime < timeDispenserOn)
        {
            stabilisationTime = EEPROM_readWord(EEPROM_ADDR_STABILISATION_TOTAL_TIME);
            fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_FAN_SPEED);
            timeDispenserOn = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_ON) * 1000;
            timeDispenserOff = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_OFF) * 1000;

            _SM_checkSensors();

            currentTime = TIME_milis();

            if (g_error == _ERROR_TEMPERATURE_CRITICAL)
            {
                GPIO_relayOff(GPIO_RELAY_HEATER);
                PWM1_setFrequency(0);
                return _RESULT_CRITICAL;
            }

            if (EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
            {
                return _RESULT_ERROR;
            }
        }
        PWM0_setDutyCycle(0);

        startTime = TIME_milis();
        currentTime = TIME_milis();
        while (currentTime - startTime < timeDispenserOff)
        {
            stabilisationTime = EEPROM_readWord(EEPROM_ADDR_STABILISATION_TOTAL_TIME);
            fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_FAN_SPEED);
            timeDispenserOn = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_ON) * 1000;
            timeDispenserOff = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_OFF) * 1000;

            _SM_checkSensors();

            currentTime = TIME_milis();

            if (g_error == _ERROR_TEMPERATURE_CRITICAL)
            {
                PWM1_setFrequency(0);
                return _RESULT_CRITICAL;
            }

            if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
            {
                return _RESULT_ERROR;
            }
        }
    }

    PWM1_setFrequency(0);

    return _RESULT_SUCCESS;
}

static uint8_t _SM_stateRunning(void)
{
    uint8_t fanSpeed = 0;
    uint32_t timeDispenserOn = 0;
    uint32_t timeDispenserOff = 0;
    uint16_t maxTemp = 0;
    uint16_t flameMax = 0;
    uint32_t flameTime = 0;
    //uint16_t criticalTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);

    uint32_t flameStartTime = 0;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    _sendState(_STATE_DEVICE_RUNNING);

    while (1)
    {
        fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_FAN_SPEED);
        timeDispenserOn = (uint32_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_ON) * 1000;
        timeDispenserOff = (uint32_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_OFF) * 1000;
        maxTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MAX);
        flameMax = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
        flameTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_TIME) * 1000;
        //uint16_t criticalTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);

        PWM1_setFrequency(fanSpeed);

        PWM0_setDutyCycle(255);
        startTime = TIME_milis();
        currentTime = TIME_milis();
        while (currentTime - startTime < timeDispenserOn)
        {
            fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_FAN_SPEED);
            timeDispenserOn = (uint32_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_ON) * 1000;
            timeDispenserOff = (uint32_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_OFF) * 1000;
            maxTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MAX);
            flameMax = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
            flameTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_TIME) * 1000;

            PWM1_setFrequency(fanSpeed);

            _SM_checkSensors();

            currentTime = TIME_milis();

            if (g_flame < flameMax)
            {
                if (!flameStartTime)
                {
                    flameStartTime = currentTime;
                }
            }
            else
            {
                flameStartTime = 0;
            }

            if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
            {
                return _RESULT_ERROR;
            }

            if (flameStartTime && currentTime - flameStartTime > flameTime)
            {
                PWM1_setFrequency(0);
                PWM0_setDutyCycle(0);
                return _RESULT_ERROR;
            }

            if (g_error == _ERROR_TEMPERATURE_CRITICAL)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                return _RESULT_CRITICAL;
            }

            if (g_temperature >= maxTemp)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                return _RESULT_SUCCESS;
            }
        }
        PWM0_setDutyCycle(0);

        startTime = TIME_milis();
        currentTime = TIME_milis();
        while (currentTime - startTime < timeDispenserOff)
        {
            fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_FAN_SPEED);
            timeDispenserOn = (uint32_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_ON) * 1000;
            timeDispenserOff = (uint32_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_OFF) * 1000;
            maxTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MAX);
            flameMax = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
            flameTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_TIME) * 1000;

            PWM1_setFrequency(fanSpeed);

            _SM_checkSensors();

            currentTime = TIME_milis();

            if (g_flame < flameMax)
            {
                if (!flameStartTime)
                {
                    flameStartTime = currentTime;
                }
            }
            else
            {
                flameStartTime = 0;
            }

            if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
            {
                return _RESULT_ERROR;
            }

            if (flameStartTime && currentTime - flameStartTime > flameTime)
            {
                PWM1_setFrequency(0);
                PWM0_setDutyCycle(0);
                return _RESULT_ERROR;
            }

            if (g_error == _ERROR_TEMPERATURE_CRITICAL)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                return _RESULT_CRITICAL;
            }

            if (g_temperature >= maxTemp)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                return _RESULT_SUCCESS;
            }
        }
    }

    return _RESULT_SUCCESS;
}

static uint8_t _SM_stateStopping(void)
{
    uint32_t flameTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_TIME) * 1000;
    uint16_t flameMax = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
    uint8_t fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_SPEED);
    uint32_t fanTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_TIME) * 1000;

    uint8_t running = 1;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    _sendState(_STATE_DEVICE_STOPPING);

    PWM0_setDutyCycle(0);
    GPIO_relayOff(GPIO_RELAY_HEATER);

    PWM1_setFrequency(fanSpeed);

    startTime = TIME_milis();
    currentTime = TIME_milis();
    while (running)
    {
        flameTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_TIME) * 1000;
        flameMax = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
        fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_SPEED);
        fanTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_TIME) * 1000;

        PWM1_setFrequency(fanSpeed);

        _SM_checkSensors();

        currentTime = TIME_milis();

        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            return _RESULT_CRITICAL;
        }

        if (g_flame <= flameMax)
        {
            if (currentTime - startTime >= flameTime)
            {
                running = 0;
            }
        }
        else
        {
            startTime = currentTime;
        }
    }

    startTime = TIME_milis();
    currentTime = TIME_milis();
    while (currentTime - startTime < fanTime)
    {
        flameTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_TIME) * 1000;
        flameMax = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
        fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_SPEED);
        fanTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_TIME) * 1000;

        _SM_checkSensors();

        currentTime = TIME_milis();

        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            return _RESULT_CRITICAL;
        }
    }

    PWM1_setFrequency(0);
    return _RESULT_SUCCESS;
}

static uint8_t _SM_stateWaiting(void)
{
    uint16_t tempMin = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MIN);

    _SM_checkSensors();

    TIME_reset();

    
    _sendState(_STATE_DEVICE_WAITING);

    if (g_error == _ERROR_TEMPERATURE_CRITICAL)
    {
        return _RESULT_CRITICAL;
    }

    if (g_error != _ERROR_NO)
    {
        return _RESULT_ERROR;
    }

    if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
    {
        return _RESULT_ERROR;
    }

    if (g_temperature < tempMin)
    {
        return _RESULT_SUCCESS;
    }

    return _RESULT_REPEATE;
}

static uint8_t _SM_stateSnail(void)
{
    _SM_checkSensors();

    if (g_error == _ERROR_TEMPERATURE_CRITICAL)
    {
        return _RESULT_CRITICAL;
    }

    if (!g_snailRunning)
    {
        PWM0_setDutyCycle(0);
        return _RESULT_SUCCESS;
    }

    return _RESULT_REPEATE;
}

static uint8_t _SM_stateCritical(void)
{
    uint8_t fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_SPEED);
    uint16_t criticalTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);
    uint16_t minTemperature = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_START);
    uint16_t waitTemp = criticalTemp - minTemperature;

    EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 0);

    TIME_reset();

    _sendState(_STATE_DEVICE_CRITICAL);
    
    PWM0_setDutyCycle(0);
    GPIO_relayOff(GPIO_RELAY_HEATER);

    PWM1_setFrequency(fanSpeed);

    while ((uint16_t)g_temperature >= waitTemp)
    {
        _SM_checkSensors();
    }

    PWM1_setFrequency(0);
    return _RESULT_SUCCESS;
}

static void _uartCallback(uint8_t *buffer, uint8_t size)
{
    if (buffer[2] == 'g')               // GET
    {
        if (buffer[3] == 'e')           // EEPROM
        {
            //cli();
            uint8_t bufferResult[] = { UART_ESC, UART_STX, 'g', 'e', 0x00, 0x00, UART_ESC, UART_ETX };
            //g_optionValue = (buffer[4] << 8) | (buffer[5]);
            //g_refreshDisplay = _REFRESH_DISPLAY_OPTION_1;
            uint16_t value = EEPROM_readWord((buffer[4] << 8) | buffer[5]);
            bufferResult[4] = (value >> 8) & 0xFF;
            bufferResult[5] = value & 0xFF;
            UART_writeBuffer(bufferResult, 8);
            //sei();
        }
    }
    else if (buffer[2] == 's')          // SET
    {
        if (buffer[3] == 'e')           // EEPROM
        {
            EEPROM_writeWord((buffer[4] << 8) | buffer[5], (buffer[6] << 8) | buffer[7]);
        }
        else if (buffer[3] == 't')      // TIME
        {
            MCP7940_now(&g_seconds, &g_minutes, &g_hours, &g_days, &g_months, &g_years);
            if (buffer[4] != 0xFF)
            {
                g_seconds = buffer[4];
            }
            if (buffer[5] != 0xFF)
            {
                g_minutes = buffer[5];
            }
            if (buffer[6] != 0xFF)
            {
                g_hours = buffer[6];
            }
            if (buffer[7] != 0xFF)
            {
                g_days = buffer[7];
            }
            if (buffer[8] != 0xFF)
            {
                g_months = buffer[8];
            }
            //g_years = (buffer[9] << 8) | buffer[10];
            uint16_t tmpYear = (buffer[9] << 8) | buffer[10];
            if (tmpYear != 0xFFFF)
            {
                g_years = tmpYear;
            }
            MCP7940_adjust(g_seconds, g_minutes, g_hours, g_days, g_months, g_years);
        }
    }
    else if (buffer[2] == 'c')          // COMMAND
    {
        if (buffer[3] == 'r')           // RUNNING
        {
            if (buffer[4])
            {
                SM_start();
            }
            else
            {
                SM_stop();
            }
        }
        else if (buffer[3] == 's')      // SNAIL
        {
            if (buffer[4])
            {
                SM_snailStart();
            }
            else
            {
                SM_snailStop();
            }
        }
    }
    else if (buffer[2] == 'r')          // RESET
    {
        if (buffer[3] == 'd')           // DEVICE
        {
            for (uint16_t i = 0x00; i < 0xFF; i++)
            {
                EEPROM_writeWord(i, 0x00);
            }
        }
    }
}

static void _sendState(uint8_t state)
{
    uint8_t buffer[] = { UART_ESC, UART_STX, 's', 'c', state, UART_ESC, UART_ETX };
    UART_writeBuffer(buffer, 7);
}

/*
ISR(TIMER0_COMPA_vect)
{
    double sensor = ds18b20_gettemp();
    UART_writeIntegerString((int)sensor);
    UART_writeString("\n");
}
*/
