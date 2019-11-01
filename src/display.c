/*
 * File name:       display.c
 * Description:     Module for display
 * Author:          Milos Zivlak <milos.zivlak@sensa-group.net>
 * Date:            2019-10-14
 * 
 */

#include "system.h"
#include "display.h"

#include <avr/io.h>

#include "driver/lcdpcf8574.h"

#define _LCD_NOBACKLIGHT 0x00
#define _LCD_BACKLIGHT   0xFF

#define _EN 6
#define _RW 5
#define _RS 4
#define _D4 0
#define _D5 1
#define _D6 2
#define _D7 3

static void _intToStr(int number, char *str);

void DISPLAY_init(void)
{
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_home();
    lcd_led(0);
}

uint8_t DISPLAY_clear(void)
{
    return lcd_clrscr();
}

uint8_t DISPLAY_home(void)
{
    return lcd_home();
}

uint8_t DISPLAY_gotoXY(uint8_t x, uint8_t y)
{
    return lcd_gotoxy(x, y);
}

uint8_t DISPLAY_showString(const char *str)
{
    return lcd_puts(str);
}

uint8_t DISPLAY_showInteger(int number)
{
    char str[10];

    _intToStr(number, str);

    return DISPLAY_showString(str);
}

static void _intToStr(int number, char *str)
{
    uint8_t n = 0;
    char tmp;

    if (number == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    if (number < 0)
    {
        str[0] = '-';
        str++;
        number *= -1;
    }

    while (number > 0)
    {
        str[n] = number % 10 + 0x30;
        number /= 10;
        n++;
    }

    str[n] = '\0';

    for (uint8_t i = 0; i < n / 2; i++)
    {
        tmp = str[i];
        str[i] = str[n - i - 1];
        str[n - i - 1] = tmp;
    }
}
