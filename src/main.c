/*
 * File name:       main.c
 * Description:     Start point of firmware
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-09
 * 
 */

#include "system.h"

#include <avr/io.h>
#include <util/delay.h>

#include "driver/uart.h"
#include "driver/pwm.h"
#include "driver/pcf8574.h"
#include "driver/ds18b20.h"
//#include "driver/ds18b20o.h"
#include "display.h"
#include "gpio.h"
#include "menu.h"

int main(void)
{
    /*
    DDRC |= (1 << PC6);

    while (1)
    {
        PORTC ^= (1 << PC6);
        _delay_ms(1000);
    }
    */

    //PWM0_init();
    //PWM0_setDutyCycle(255);
    //PWM0_setDutyCycle(100);

    /*
    while (1)
    {
        for (uint16_t i = 100; i < 255; i += 25)
        {
            PWM0_setDutyCycle(i);
            _delay_ms(500);
        }
    }

    while (1);


    while (1)
    {
        PWM0_setDutyCycle(0);
        _delay_ms(1000);
        PWM0_setDutyCycle(0xFF / 4);
        _delay_ms(1000);
        PWM0_setDutyCycle(0xFF / 2);
        _delay_ms(1000);
        PWM0_setDutyCycle(0xFF);
        _delay_ms(1000);
    }
    */

    /*
    DDRB |= (1 << PB7);

    while (1)
    {
        PORTB ^= (1 << PB7);
        _delay_ms(10);
    }
    */

    // 10 - 20 (maksimum je 1, od 10 do 20 se vidi mala razlika, posle toga je sve isto)
    //PWM1_init();
    //PWM1_setFrequency(10);

    //DDRB &= ~(1 << PB7);

    //while (1);

    /*
     * 0x3C - Relays
     * 0x38 - Display
     */

    /*
    DDRB |= (1 << PB7);
    while (1)
    {
        PORTB ^= (1 << PB7);
        _delay_ms(1000);
    }
    */

    /*
    DDRE |= (1 << PE6);

    while (1)
    {
        PORTE |= (1 << PE6);
        _delay_ms(1);
        PORTE &= ~(1 << PE6);
        _delay_ms(1);
    }

    while (1);
    */

    UART_init();
    UART_writeString("RADI\n");

    //DS18B20O_init();

    while (1)
    {
        //int tmp = (int)DS18B20O_readT1();
        int tmp = (int)ds18b20_gettemp();
        UART_writeIntegerString(tmp);
        UART_writeString("\n");
        //UART_write(tmp);
    }

    /*
    while (1)
    {
        int tmp = (int)ds18b20_gettemp();
        //UART_writeIntegerString(tmp);
        UART_write(tmp);
        _delay_ms(500);
    }
    */

    /*
    DISPLAY_init();
    DISPLAY_showString("ZI JE CAR");
    DISPLAY_gotoXY(0, 1);
    DISPLAY_showString("SENSA");
    DISPLAY_gotoXY(0, 2);
    DISPLAY_showString("http://sensa-group.n");
    DISPLAY_gotoXY(0, 3);
    DISPLAY_showString(":D");

    MENU_init();
    */

    while (1);

    /*
    ONEWIRE_init();

    uint8_t addr[8];

    UART_writeString("START\n");
    while (ONEWIRE_search(addr))
    {
        UART_writeString("USAO\n");
        UART_writeBuffer(addr, 8);
    }
    UART_writeString("KRAJ\n");
    */

    /*
    pcf8574_init();
    GPIO_init();

    while (1)
    {
        GPIO_relayOn(GPIO_RELAY_HEATER);
        _delay_ms(1000);
        GPIO_relayOff(GPIO_RELAY_HEATER);
        _delay_ms(1000);
    }
    */

    /*
    DISPLAY_init();
    DISPLAY_showString("ZI JE CAR");
    DISPLAY_gotoXY(0, 1);
    DISPLAY_showString("SENSA");
    DISPLAY_gotoXY(0, 2);
    DISPLAY_showString("http://sensa-group.n");
    DISPLAY_gotoXY(0, 3);
    DISPLAY_showString(":D");
    */

    //

    while (1);

    //while (1);

    //UART_writeString("UART RADI\n");

    /*
    I2C_init();
    PCF8574_init();
    GPIO_init();

    while (1)
    {
        GPIO_relayOn(GPIO_RELAY_HEATER);
        //UART_writeString("ON\n");
        _delay_ms(1000);
        GPIO_relayOff(GPIO_RELAY_HEATER);
        //UART_writeString("OFF\n");
        _delay_ms(1000);
    }
    */

    /*
    pcf8574_init();
    GPIO_init();

    while (1)
    {
        GPIO_relayOn(GPIO_RELAY_HEATER);
        _delay_ms(1000);
        GPIO_relayOff(GPIO_RELAY_HEATER);
        _delay_ms(1000);
    }
    */

    while (1);

    return 0;
}
