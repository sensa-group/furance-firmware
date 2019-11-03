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
#include "gpio.h"
#include "display.h"
#include "menu.h"

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

static double g_temperature = 0.0;
static double g_flame = 0;

volatile static uint8_t g_snailRunning = 0;

static uint8_t g_lastError;
static uint8_t g_error;

void SM_init(void)
{
    g_currentState = _STATE_STOPPED;
    g_error = _ERROR_NO;
    g_lastError = g_error;

    UART_init();                                                            // For debugging

    pcf8574_init();                                                         // PCF8574 init

    GPIO_init();                                                            // Initialize GPIO (digital input/output pins)

    PWM0_init();                                                            // Initialize PWM0 (DC Motor)
    PWM1_init();                                                            // Initialize PWM1 (AC Motor -> FAN)
    PWM2_init();                                                            // Initialize PWM2 ()

    ADC_init();                                                             // Initialize ADC foto resistor

    sei();                                                                  // Enable interrupts

    DISPLAY_init();                                                         // Initialize display

    MENU_init();                                                            // Initialize menu

    /*
    TCNT0 = 0;
    OCR0A = 0;
    OCR0B = 0;

    //TCCR1A = (1 << COM1A1);
    TCCR0B = (1<< CS00) | (1 << WGM02);
    TIMSK0 = (1 << OCIE0A);

    TCNT0 = 0;

    OCR0A = 16000;
    */

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
        flame = (uint16_t)SYSTEM_MAP(flame, 1023.0, 100.0, 0.0, 100.0);

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

    UART_writeString("SM: Initialized\n");
}

void SM_exec(void)
{
    uint8_t result;
    //uint8_t lastState;

    while (1)
    {
        /*
        _SM_checkSensors();
        _SM_refreshDisplay();

        if (g_snailRunning)
        {
            g_currentState = _STATE_SNAIL;
            g_stateInitialized = 0;
            g_stateStartTime = TIMER0_millis();
        }

        if (g_currentState == _STATE_STOPPED && EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
        {
            g_currentState = _STATE_STARTING;
            g_stateInitialized = 0;
            g_stateStartTime = TIMER0_millis();
        }
        else if (g_currentState != _STATE_STOPPED && !EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
        {
            if (g_currentState == _STATE_WAITING)
            {
                g_currentState = _STATE_STOPPED;
            }
            if (g_currentState != _STATE_WAITING)
            {
                g_currentState = _STATE_STOPPED;
            }
            SYSTEM_WRITE_LOW(SYSTEM_PORTC, 0);
            GPIO_relayOff(GPIO_RELAY_HEATER);
            PWM1_setDutyCycleCH0(0);
            g_stateInitialized = 0;
            g_stateStartTime = TIMER0_millis();
        }
        */

        if (g_error == _ERROR_NO)
        {
            UART_writeString("State: ");
            UART_writeIntegerString(g_currentState);
            UART_writeString("\n");
            cli();
            switch (g_currentState)
            {
                case _STATE_STOPPED:
                    MENU_refreshError("STOP", "");
                    break;
                case _STATE_STARTING:
                    MENU_refreshError("POTPALA", "");
                    break;
                case _STATE_STABILISATION:
                    MENU_refreshError("STAB.", "");
                    break;
                case _STATE_RUNNING:
                    MENU_refreshError("RAD", "");
                    break;
                case _STATE_STOPPING:
                    MENU_refreshError("GASENJE", "");
                    break;
                case _STATE_WAITING:
                    MENU_refreshError("CEKANJE", "");
                    break;
                default:
                    MENU_refreshError("", "");
                    break;
            }
            sei();
        }
        
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
    uint16_t temperatureCritical = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);
    uint16_t temperatureStart = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_START);
    uint16_t flame = ADC_read(0b111);
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

    i = 0;
    double flame2 = 0;
    for (i = 0; i < 10; i++)
    {
        flame2 += ADC_read(0b111);
    }
    flame2 /= 10.0;
    flame = (uint16_t)flame2;

    if (temperature > temperatureCritical)
    {
        if (g_error != _ERROR_TEMPERATURE_CRITICAL)
        {
            g_error = _ERROR_TEMPERATURE_CRITICAL;
        }
    }

    /*
    UART_writeString("R: ");
    UART_writeIntegerString(flame);
    UART_writeString("\n");
    */

    /*
    if (flame == 1023)
    {
        g_error = _ERROR_FLAME_DISCONNECTED;
    }
    */

    //flame = SYSTEM_MAP(flame, 1020, 100, 0, 100);
    //flame = 100 - SYSTEM_MAP(flame, 100, 1023, 0, 100);
    flame = (uint16_t)SYSTEM_MAP(flame, 1023.0, 100.0, 0.0, 100.0);

    if (g_flame != flame || g_temperature != temperature)
    {
        /*
        UART_writeString("T: ");
        UART_writeIntegerString(temperature);
        UART_writeString("\n");
        UART_writeString("F: ");
        UART_writeIntegerString(flame);
        UART_writeString("\n");
        */
        cli();
        MENU_refreshSensorValue((uint16_t)temperature, (uint16_t)flame);
        sei();
    }

    g_flame = flame;
    g_temperature = temperature;

    _SM_checkError();
}

static void _SM_checkError(void)
{
    if (g_error != g_lastError)
    {
        cli();
        switch (g_error)
        {
            case _ERROR_NO:
                MENU_refreshError("", "");
                GPIO_buzzerOff();
                break;
            case _ERROR_TEMPERATURE_CRITICAL:
                MENU_refreshError("G:RAD", "KRIT TEM");
                GPIO_buzzerOn();
                break;
            case _ERROR_TEMPERATURE_DISCONNECTED:
                MENU_refreshError("G:SENZOR", "TEMP");
                GPIO_buzzerOn();
                break;
            case _ERROR_FLAME_DISCONNECTED:
                MENU_refreshError("G:SENZOR", "OKO");
                GPIO_buzzerOn();
                break;
        }
        sei();
    }
    g_lastError = g_error;
}

static uint8_t _SM_stateStopped(void)
{
    _SM_checkSensors();

    TIME_reset();

    if (g_error == _ERROR_TEMPERATURE_CRITICAL)
    {
        uint16_t startTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_START);
        if (g_temperature >= startTemp)
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
    uint8_t fan1Speed = SYSTEM_MAP((uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_SPEED), 0, 100, 1, 20);
    uint16_t fan1Time = EEPROM_readWord(EEPROM_ADDR_STARTING_FAN_TIME) * 1000;
    uint16_t dispenserTime = EEPROM_readWord(EEPROM_ADDR_STARTING_DISPENSER_TIME) * 1000;
    uint32_t heaterTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STARTING_HEATER_TIME) * 1000;
    uint8_t fan2Speed = SYSTEM_MAP((uint8_t)EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_SPEED), 0, 100, 1, 20);
    uint16_t fan2WaitingTime = EEPROM_readWord(EEPROM_ADDR_STARTING_FAN2_WAITING_TIME) * 1000;
    uint16_t flameMin = EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_MAX);
    uint16_t flameTime = (uint16_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FLAME_TIME) * 1000;

    uint8_t fanStarted = 0;
    uint32_t flameStartTime = 0;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    UART_writeString("STARTING: Fan started\n");
    PWM1_setFrequency(fan1Speed);
    while (currentTime - startTime < fan1Time)
    {
        //UART_writeString("STARTING: Fan 01\n");
        _SM_checkSensors();
        //UART_writeString("STARTING: Fan 02\n");
        //_delay_ms(10);
        //UART_writeString("STARTING: Fan 03\n");
        //UART_writeString("STARTING: Fan 04\n");

        currentTime = TIME_milis();
        
        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            UART_writeString("STARTING: Teperature critical\n");
            return _RESULT_CRITICAL;
        }

        if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
        {
            return _RESULT_ERROR;
        }
    }
    PWM1_setFrequency(0);
    UART_writeString("STARTING: Fan stopped\n");

    PWM0_setDutyCycle(255);
    UART_writeString("STARTING: Dispenser started\n");
    startTime = TIME_milis();
    currentTime = TIME_milis();
    while (currentTime - startTime < dispenserTime)
    {
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
    UART_writeString("STARTING: Dispenser started\n");

    GPIO_relayOn(GPIO_RELAY_HEATER);
    UART_writeString("STARTING: Heater started\n");
    startTime = TIME_milis();
    currentTime = TIME_milis();
    while (currentTime - startTime < heaterTime)
    {
        _SM_checkSensors();

        currentTime = TIME_milis();

        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            UART_writeString("STARTING: Critical\n");
            return _RESULT_CRITICAL;
        }

        if (!EEPROM_readWord(EEPROM_ADDR_SYSTEM_RUNNING))
        {
            return _RESULT_ERROR;
        }

        if (currentTime - startTime >= fan2WaitingTime && !fanStarted)
        {
            UART_writeString("STARTING: Fan started\n");
            PWM1_setFrequency(fan2Speed);
            fanStarted = 1;
        }

        if (g_flame > flameMin)
        {
            if (!flameStartTime)
            {
                UART_writeString("STARTING: Flame detected\n");
                flameStartTime = currentTime;
            }
        }
        else
        {
            UART_writeString("STARTING: Flame not detected\n");
            flameStartTime = 0;
        }

        if (flameStartTime && currentTime - flameStartTime > flameTime)
        {
            PWM1_setFrequency(0);
            GPIO_relayOff(GPIO_RELAY_HEATER);
            UART_writeString("STARTING: Heater stopped\n");
            UART_writeString("STARTING: Fan started\n");
            UART_writeString("STARTING: Success\n");
            return _RESULT_SUCCESS;
        }
    }
    PWM1_setFrequency(0);
    GPIO_relayOff(GPIO_RELAY_HEATER);
    UART_writeString("STARTING: Error\n");

    return _RESULT_ERROR;
}

static uint8_t _SM_stateStabilisation(void)
{
    uint16_t stabilisationTime = EEPROM_readWord(EEPROM_ADDR_STABILISATION_TOTAL_TIME);
    uint8_t fanSpeed = (uint8_t)SYSTEM_MAP(EEPROM_readWord(EEPROM_ADDR_STABILISATION_FAN_SPEED), 0, 100.0, 1.0, 20.0);
    uint16_t timeDispenserOn = EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_ON) * 1000;
    uint16_t timeDispenserOff = EEPROM_readWord(EEPROM_ADDR_STABILISATION_DISPENSER_TIME_OFF) * 1000;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    PWM1_setFrequency(fanSpeed);

    for (uint16_t i = 0; i < stabilisationTime; i++)
    {
        PWM0_setDutyCycle(255);
        startTime = TIME_milis();
        currentTime = TIME_milis();
        while (currentTime - startTime < timeDispenserOn)
        {
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
    uint8_t fanSpeed = (uint8_t)EEPROM_readWord(EEPROM_ADDR_RUNNING_FAN_SPEED);
    uint16_t timeDispenserOn = EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_ON) * 1000;
    uint16_t timeDispenserOff = EEPROM_readWord(EEPROM_ADDR_RUNNING_DISPENSER_TIME_OFF) * 1000;
    uint16_t maxTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MAX);
    uint16_t flameMax = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_MIN);
    uint16_t flameTime = EEPROM_readWord(EEPROM_ADDR_STARTING_FLAME_TIME) * 1000;
    //uint16_t criticalTemp = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_CRITICAL);

    uint32_t flameStartTime = 0;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    PWM1_setFrequency(fanSpeed);

    while (1)
    {
        PWM0_setDutyCycle(255);
        startTime = TIME_milis();
        currentTime = TIME_milis();
        UART_writeString("RUN: Dispenser started\n");
        while (currentTime - startTime < timeDispenserOn)
        {
            _SM_checkSensors();

            currentTime = TIME_milis();

            if (g_flame < flameMax)
            {
                if (!flameStartTime)
                {
                    UART_writeString("RUN: Flame detected\n");
                    flameStartTime = currentTime;
                }
            }
            else
            {
                UART_writeString("RUN: Flame not detected\n");
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
                UART_writeString("RUN: Heater stopped\n");
                UART_writeString("RUN: Fan started\n");
                UART_writeString("RUN: Error\n");
                return _RESULT_ERROR;
            }

            if (g_error == _ERROR_TEMPERATURE_CRITICAL)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                UART_writeString("RUN: Critical\n");
                return _RESULT_CRITICAL;
            }

            if (g_temperature >= maxTemp)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                UART_writeString("RUN: Max temp\n");
                return _RESULT_SUCCESS;
            }
        }
        PWM0_setDutyCycle(0);
        UART_writeString("RUN: Dispenser stopped\n");

        startTime = TIME_milis();
        currentTime = TIME_milis();
        while (currentTime - startTime < timeDispenserOff)
        {
            _SM_checkSensors();

            currentTime = TIME_milis();

            if (g_flame < flameMax)
            {
                if (!flameStartTime)
                {
                    UART_writeString("RUN: Flame detected\n");
                    flameStartTime = currentTime;
                }
            }
            else
            {
                UART_writeString("RUN: Flame not detected\n");
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
                UART_writeString("RUN: Heater stopped\n");
                UART_writeString("RUN: Fan started\n");
                UART_writeString("RUN: Error\n");
                return _RESULT_ERROR;
            }

            if (g_error == _ERROR_TEMPERATURE_CRITICAL)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                UART_writeString("RUN: Critical\n");
                return _RESULT_CRITICAL;
            }

            if (g_temperature >= maxTemp)
            {
                PWM0_setDutyCycle(0);
                PWM1_setFrequency(0);
                UART_writeString("RUN: Max temp\n");
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
    uint8_t fanSpeed = (uint8_t)SYSTEM_MAP(EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_SPEED), 0.0, 100.0, 10.0, 20.0);
    uint32_t fanTime = (uint32_t)EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_TIME) * 1000;

    uint8_t running = 1;

    uint32_t startTime = TIME_milis();
    uint32_t currentTime = TIME_milis();

    PWM0_setDutyCycle(0);
    GPIO_relayOff(GPIO_RELAY_HEATER);

    PWM1_setFrequency(fanSpeed);

    startTime = TIME_milis();
    currentTime = TIME_milis();
    UART_writeString("STOPPING: Flame stopping\n");
    while (running)
    {
        _SM_checkSensors();

        currentTime = TIME_milis();

        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            UART_writeString("STOPPING: Critical\n");
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
    UART_writeString("STOPPING: Flame stopped\n");

    startTime = TIME_milis();
    currentTime = TIME_milis();
    UART_writeString("STOPPING: Start waiting\n");
    while (currentTime - startTime < fanTime)
    {
        _SM_checkSensors();

        currentTime = TIME_milis();

        if (g_error == _ERROR_TEMPERATURE_CRITICAL)
        {
            UART_writeString("STOPPING: Critical\n");
            return _RESULT_CRITICAL;
        }
    }
    UART_writeString("STOPPING: Stop waiting\n");

    PWM1_setFrequency(0);
    return _RESULT_SUCCESS;
}

static uint8_t _SM_stateWaiting(void)
{
    uint16_t tempMin = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_MIN);

    _SM_checkSensors();

    TIME_reset();

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
    uint8_t fanSpeed = (uint8_t)SYSTEM_MAP(EEPROM_readWord(EEPROM_ADDR_STOPPING_FAN_SPEED), 0.0, 100.0, 10.0, 20.0);
    uint16_t minTemperature = EEPROM_readWord(EEPROM_ADDR_GLOBAL_TEMP_START);

    EEPROM_writeWord(EEPROM_ADDR_SYSTEM_RUNNING, 0);

    TIME_reset();
    
    PWM0_setDutyCycle(0);
    GPIO_relayOff(GPIO_RELAY_HEATER);

    PWM1_setFrequency(fanSpeed);

    UART_writeString("CRITICAL: Start waiting\n");
    while (1)
    {
        _SM_checkSensors();

        if (g_temperature < minTemperature)
        {
            break;
        }
    }
    UART_writeString("CRITICAL: Stop waiting\n");

    PWM1_setFrequency(0);
    return _RESULT_SUCCESS;
}

/*
ISR(TIMER0_COMPA_vect)
{
    double sensor = ds18b20_gettemp();
    UART_writeIntegerString((int)sensor);
    UART_writeString("\n");
}
*/
