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
#include "driver/max6675.h"
#include "driver/hx711.h"
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

    UART_writeString("START\n");

    HX711_init(128);
    //Averagebegin(average);
    HX711_set_scale(1.f/*242300.88*/);
    //HX711_set_offset(0.f);
    HX711_set_gain(128);
    HX711_tare(10);
    double tare_point_128 = HX711_get_offset();
    double calibration_128 = 222138.f;//220235.f;
    _delay_ms(500);

    double derivative = 0.0, ddrivative = 0.0, change_rate = 0.00100;
    double current_weight_128_dc, previous_weight_128, system_weight, offset_weight, pprevious_weight_128;

    double current_reading, current_weight_128, current_weight_32, adjustment_weight_0 = 0, system_lock_weight_array[5] = {0};
    int filterSamples = 20;
    float ptr[20];

    /*
    for(int i=0;i<10;i++){
        current_weight_128 = HX711_read_average(10) - tare_point_128;
        current_weight_128 = current_weight_128/calibration_128;
        if(i==8){
            pprevious_weight_128 = current_weight_128;
        }
    }
    */

    previous_weight_128 = current_weight_128;
    uint8_t trend = 0, frozen_w = 0, system_lock = 0;
    double sum_weight = 0.0;
    int count = 0;

    while (1)
    {
        current_weight_128 = HX711_read_average(10);
        UART_writeIntegerString(current_weight_128);
        UART_writeString("\n");
        _delay_ms(100);
        continue;

        current_weight_128 = HX711_read_average(10) - tare_point_128;
        current_weight_128 = current_weight_128/calibration_128;
    //    current_weight_128 = sum_weight/average;
        derivative = (current_weight_128 - previous_weight_128);//(time_mid - time_start)*100000;
        ddrivative = current_weight_128 + pprevious_weight_128 - 2*previous_weight_128;
        pprevious_weight_128 = previous_weight_128;
        previous_weight_128 = current_weight_128;
        UART_writeString("S Deri = ");
        UART_writeIntegerString((int)derivative);
        UART_writeString("\n");
        UART_writeString("D Deri = ");
        UART_writeIntegerString((int)ddrivative);
        UART_writeString("\n");
        UART_writeString("Weight = ");
        UART_writeIntegerString((int)current_weight_128);
        UART_writeString("\n");
        if(-change_rate < derivative && derivative < change_rate){
            trend = 1;
            if(frozen_w == 0 /*&& current_weight_128 < 0.0050*/){
                offset_weight = current_weight_128;
            }
            else if(frozen_w == 1 /*&& current_weight_128 > 0.00050*/){
                offset_weight = offset_weight;
            }
            UART_writeString("offset weight = ");
            UART_writeIntegerString((int)offset_weight);
            UART_writeString("\n");

        }
        else if(derivative >= change_rate){
            system_lock = 1;
            frozen_w = 1;
            system_lock = 1;
            UART_writeString("offset weight = ");
            UART_writeIntegerString((int)offset_weight);
            UART_writeString("\n");
            /*for(int i=0;i<5;i++){
                system_lock_weight_array[i] = HX711_read_average(10) - tare_point_128;
                system_lock_weight_array[i] = system_lock_weight_array[i]/calibration_128;
                fprintf(stdout,"array = %.5f", system_lock_weight_array[i]);
                printf(" , ");
            }*/
            trend = 0;
        }
        else{
            frozen_w = 0;
            UART_writeString("frozen_status = ");
            UART_writeIntegerString((int)offset_weight);
            UART_writeString("\n");
            system_lock = 0;
            trend = 0;
        }
        if(current_weight_128 < 0.00050 && current_weight_128 > -0.00050){
            current_weight_128_dc = 0.0;
        }
        else /*if(current_weight_128 < -0.00050 && current_weight_128 > 0.00050 )*/{
            current_weight_128_dc = current_weight_128 - offset_weight;
        }
        if(frozen_w == 1 && trend == 0){
            count++;
            UART_writeString("count = ");
            UART_writeIntegerString((int)offset_weight);
            UART_writeString("\n");
            //if(count == 2){
                system_weight = current_weight_128_dc;
            //}
        }
        else if(frozen_w == 0){
            count = 0;
            system_weight = current_weight_128_dc;
        }
        //if(system_lock == true && trend == false){
          //  system_weight = current_weight_128;
            //fprintf(stdout,"System Lock = %.5f", current_weight_128_dc);
            //printf(" System Lock trend false");
        //}

        UART_writeString("current weight 128 = ");
        UART_writeIntegerString((int)current_weight_128);
        UART_writeString("\n");
        UART_writeString("current weight 128 = ");
        UART_writeIntegerString((int)current_weight_128_dc);
        UART_writeString("\n");
        UART_writeString("system_weight = ");
        UART_writeIntegerString((int)system_weight);
        UART_writeString("\n");
        UART_writeString("calibration 128 = ");
        UART_writeIntegerString((int)calibration_128);
        UART_writeString("\n");
        UART_writeString("tare_point_128 = ");
        UART_writeIntegerString((int)tare_point_128);
        UART_writeString("\n");

        /*
        char temp = USART_Receive(NULL);
        if(temp == 'a'){
            tare_point_128 = tare_point_128 + 1;
        }
        else if (temp == 'z'){
            tare_point_128 = tare_point_128 - 1;
        }
        else if (temp == 'x'){
            calibration_128 = calibration_128 - 1;
        }
        else if (temp == 's'){
            calibration_128 = calibration_128 + 1;
        }
        */
        //HX711_power_down();
    }

    /*
    MAX6675_init();
    while (1)
    {
        //int tmp = (int)DS18B20O_readT1();
        int tmp = (int)MAX6675_readCelsius();
        UART_writeIntegerString(tmp);
        UART_writeString("\n");
        _delay_ms(500);
        //UART_write(tmp);
    }
    */

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
