/**
    * Custom library for showing measurements to 16x2 LCD display (for BPC-DE2 project)
    * @author Jakub Kovac
    * 27. 11. 2024
*/
// -- Includes --
#include "screen.h"     // Header file
#include <math.h>       // C library. fabs()
#include <stdlib.h>     // C library. Needed for number conversion
#include <lcd.h>        // Peter Fleury's LCD library
#include <uart.h>       // Peter Fleury's UART library (for debug messages if needed)

// -- Global variables --
struct angleStruct {
    int8_t aInt;        // Integer part
    uint8_t aDec;       // Decimal part
    uint8_t aSign;      // Sign (0 positive or 1 negative)
} angleIntegers;

struct heightStruct{
    int8_t hInt;        // Integer part
    uint8_t hDec;       // Decimal part
    uint8_t hSign;      // Sign (0 positive or 1 negative)
} heightIntegers;

char print[2];          // for itoa()

// -- Functions definitiosn --
/**
* Function: lcdInit
* Purpose:  Initializes the 16x2 LCD screen and adds custom characters.
* @return  None
*/
void lcdInit(void)
{
    // Local variables - custom characters
    uint8_t leftBar[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0};
    uint8_t leftCenterBar[8] = {0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x0};
    uint8_t rightCenterBar[8] = {0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x0};
    uint8_t rightBar[8] = {0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0};
    uint8_t center[8] = {0xa,0x0,0x0,0x0,0x0,0x0,0xa,0x0};
    // Initialize LCD
    lcd_init(LCD_DISP_ON);
    lcd_custom_char(0, leftBar);
    lcd_custom_char(1, leftCenterBar);
    lcd_custom_char(2, rightCenterBar);
    lcd_custom_char(3, rightBar);
    lcd_custom_char(4, center);
    lcd_clrscr();
    lcd_home();
}
/**
    * Function: updateAngle
    * Purpose:  Updates the screen with new angle information and graphical represantation.
    * @param angle Value of angle to be written and represented on screen
    * @return None
*/
void updateAngle(float angle) 
{
    // Converting float to decimal parts with sign flag
    if(angle < 0) angleIntegers.aSign = 1; 
    else angleIntegers.aSign = 0;
    angleIntegers.aInt = fabs(angle);
    angleIntegers.aDec = (fabs(angle) - angleIntegers.aInt) * 10;

    // Writing number to LCD
    if(angleIntegers.aSign == 1) // Negative number
    {
        if(angleIntegers.aInt >= 10) // less than or equal to -10
        {
            lcd_gotoxy(10, 0);
            lcd_putc('-');
            itoa(angleIntegers.aInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(13, 0);
            lcd_putc(',');
            lcd_gotoxy(14, 0);
            itoa(angleIntegers.aDec, print, 10);
            lcd_puts(print);
            lcd_gotoxy(15, 0);
            lcd_putc(0xDF);
        }
        else // greater than or equal to -9
        {
            lcd_gotoxy(10, 0);
            lcd_puts(" -");
            itoa(angleIntegers.aInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(13, 0);
            lcd_putc(',');
            lcd_gotoxy(14, 0);
            itoa(angleIntegers.aDec, print, 10);
            lcd_puts(print);
            lcd_gotoxy(15, 0);
            lcd_putc(0xDF);
        }
    }
    else // Positive number
    {
        if(angleIntegers.aInt >= 10) // greater than or equal to 10
        {
            lcd_gotoxy(10, 0);
            lcd_putc(' ');
            itoa(angleIntegers.aInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(13, 0);
            lcd_putc(',');
            lcd_gotoxy(14, 0);
            itoa(angleIntegers.aDec, print, 10);
            lcd_puts(print);
            lcd_gotoxy(15, 0);
            lcd_putc(0xDF);
        }
        else // less than or equal to 9
        {
            lcd_gotoxy(10, 0);
            lcd_puts("  ");
            itoa(angleIntegers.aInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(13, 0);
            lcd_putc(',');
            lcd_gotoxy(14, 0);
            itoa(angleIntegers.aDec, print, 10);
            lcd_puts(print);
            lcd_gotoxy(15, 0);
            lcd_putc(0xDF);
        }
    }
}
/**
    * Function: updateAngleBar
    * Purpose:  Updates the screen with new angle graphical represantation.
    * @param height Value of angle
    * @return None
*/
void updateAngleBar(float angle)
{
    // Local variables
    int8_t barPosition = 0;
    // Calculating bar position and choosing correct character
    if(angleIntegers.aInt >= 6) barPosition = (angleIntegers.aInt-6)/30 + 1;
    else barPosition = 0;

    // Clear the bar
    lcd_gotoxy(1, 0);
    lcd_puts("   ");
    lcd_putc(4);
    lcd_puts("   ");

    // New bar position (character position)
    if(angleIntegers.aSign == 0) lcd_gotoxy(4+barPosition, 0);
    else lcd_gotoxy(4-barPosition, 0);
    // Choose new character and show it
    if(angleIntegers.aInt < 6 && angleIntegers.aInt > -6) lcd_putc('I');
    else if((angle <= (-6)-(barPosition-1)*30 && angle > (-12)-(barPosition-1)*30) || (angle >= 30+(barPosition-1)*30 && angle < 36+(barPosition-1)*30)) lcd_putc(3);
    else if((angle <= (-12)-(barPosition-1)*30 && angle > (-18)-(barPosition-1)*30) || (angle >= 24+(barPosition-1)*30 && angle < 30+(barPosition-1)*30)) lcd_putc(2);
    else if((angle <= (-18)-(barPosition-1)*30 && angle > (-24)-(barPosition-1)*30) || (angle >= 18+(barPosition-1)*30 && angle < 24+(barPosition-1)*30)) lcd_putc('|');
    else if((angle <= (-24)-(barPosition-1)*30 && angle > (-30)-(barPosition-1)*30) || (angle >= 12+(barPosition-1)*30 && angle < 18+(barPosition-1)*30)) lcd_putc(1);
    else if((angle <= (-30)-(barPosition-1)*30 && angle > (-36)-(barPosition-1)*30) || (angle >= 6+(barPosition-1)*30 && angle < 12+(barPosition-1)*30)) lcd_putc(0);
}
/**
    * Function: updateHeight
    * Purpose:  Updates the screen with new height information.
    * @param height Value of height to be written on LCD
    * @return None
*/
void updateHeight(float height) 
{
    if(height < 0) heightIntegers.hSign = 1; 
    else heightIntegers.hSign = 0;
    heightIntegers.hInt = fabs(height);
    heightIntegers.hDec = (fabs(height) - heightIntegers.hInt) * 10;

    // Writing number to LCD  
    if(heightIntegers.hSign == 1) // Negative number
    {
        if(heightIntegers.hInt >= 10) // less than or equal to -10
        {
            lcd_gotoxy(10, 1);
            lcd_putc('-');
            itoa(heightIntegers.hInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(14, 1);
            itoa(heightIntegers.hDec, print, 10);
            lcd_puts(print);
        }
        else // greater than or equal to -9
        {
            lcd_gotoxy(10, 1);
            lcd_puts(" -");
            itoa(heightIntegers.hInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(14, 1);
            itoa(heightIntegers.hDec, print, 10);
            lcd_puts(print);
        }
    }
    else // Positive number
    {
        if(heightIntegers.hInt >= 10) // greater than or equal to 10
        {
            lcd_gotoxy(10, 1);
            lcd_putc(' ');
            itoa(heightIntegers.hInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(14, 1);
            itoa(heightIntegers.hDec, print, 10);
            lcd_puts(print);
        }
        else // less than or equal to 9
        {
            lcd_gotoxy(10, 1);
            lcd_puts("  ");
            itoa(heightIntegers.hInt, print, 10);
            lcd_puts(print);
            lcd_gotoxy(14, 1);
            itoa(heightIntegers.hDec, print, 10);
            lcd_puts(print);
        }
    }
}