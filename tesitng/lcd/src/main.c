/*
  Simulating angle and height data update to LCD.
  Jakub Kovac
  24. 11. 2024
*/

// -- Includes --
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversion
#include <stdio.h>          // C library. sprintf 
#include <math.h>           // C library. fabs()

// -- Global variables -- 
// "Flags"
volatile uint8_t updateAngle = 0;
volatile uint8_t updateHeight = 0;

char print[3]; // for itoa
// Measured data
float angleFloat = 0;
float heightFloat = 0;

// Structs for converting float nubmer to decimal
struct Angle_structure {
    int8_t aInt;    // Integer part
    uint8_t aDec;   // Decimal part
    uint8_t aSign;  // Sign (0 positive or 1 negative)
} angle;

struct Height_structure {
    int8_t hInt;    // Integer part
    uint8_t hDec;   // Decimal part
    uint8_t hSign;  // Sign (0 positive or 1 negative)
} height;

// -- Function definitions --
/*
 * Function: updAngle
 * Purpose:  Update the screen with new angle information and graphical represantation.
 * Returns:  none
*/
void updAngle() {
  static int8_t barPosition = 0;
  // Converting float to decimal parts with sign flag
  if (angleFloat < 0) 
  {
    angle.aSign = 1; 
  }
  else
  {
    angle.aSign = 0;
  }
  angle.aInt = fabs(angleFloat);
  angle.aDec = (fabs(angleFloat) - angle.aInt) * 10;
  // Writing number to LCD
  if (angle.aSign == 1) // Negative number
  {
    if(angle.aInt >= 10)
    {
      lcd_gotoxy(10, 0);
      lcd_putc('-');
      itoa(angle.aInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 0);
      itoa(angle.aDec, print, 10);
      lcd_puts(print);
    }
    else
    {
      lcd_gotoxy(10, 0);
      lcd_puts(" -");
      itoa(angle.aInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 0);
      itoa(angle.aDec, print, 10);
      lcd_puts(print);
    }
  }
  else // Positive number
  {
      if(angle.aInt >= 10)
    {
      lcd_gotoxy(10, 0);
      lcd_putc(' ');
      itoa(angle.aInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 0);
      itoa(angle.aDec, print, 10);
      lcd_puts(print);
    }
    else
    {
      lcd_gotoxy(10, 0);
      lcd_puts("  ");
      itoa(angle.aInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 0);
      itoa(angle.aDec, print, 10);
      lcd_puts(print);
    }
  }
  // Calculating bar position and choosing correct character
  if(angle.aInt >= 6)
  { 
    barPosition = (angle.aInt-6)/30 + 1;
  }
  else
  {
    barPosition = 0;
  }
  // Clear
  lcd_gotoxy(1, 0);
  lcd_puts("   ");
  lcd_putc(4);
  lcd_puts("   ");

  if (angle.aSign == 0)
  {
    lcd_gotoxy(4+barPosition, 0);
  }
  else
  {
    lcd_gotoxy(4-barPosition, 0);
  }
  if(angle.aInt < 6 && angle.aInt > -6)
  {
    lcd_putc('I');
  }
  else if((angleFloat <= (-6)-(barPosition-1)*30 && angleFloat > (-12)-(barPosition-1)*30) || (angleFloat >= 30+(barPosition-1)*30 && angleFloat < 36+(barPosition-1)*30)) 
  {
    lcd_putc(3);
  }
  else if((angleFloat <= (-12)-(barPosition-1)*30 && angleFloat > (-18)-(barPosition-1)*30) || (angleFloat >= 24+(barPosition-1)*30 && angleFloat < 30+(barPosition-1)*30)) 
  {
    lcd_putc(2);
  }
  else if((angleFloat <= (-18)-(barPosition-1)*30 && angleFloat > (-24)-(barPosition-1)*30) || (angleFloat >= 18+(barPosition-1)*30 && angleFloat < 24+(barPosition-1)*30))
  { 
    lcd_putc('|');
  }
  else if((angleFloat <= (-24)-(barPosition-1)*30 && angleFloat > (-30)-(barPosition-1)*30) || (angleFloat >= 12+(barPosition-1)*30 && angleFloat < 18+(barPosition-1)*30)) 
  {
    lcd_putc(1);
  }
  else if((angleFloat <= (-30)-(barPosition-1)*30 && angleFloat > (-36)-(barPosition-1)*30) || (angleFloat >= 6+(barPosition-1)*30 && angleFloat < 12+(barPosition-1)*30)) 
  {
    lcd_putc(0);
  }
  updateAngle--; // Update completed
}

/*
 * Function: updHeight
 * Purpose:  Update the screen with new height information and graphical represantation.
 * Returns:  none
*/
void updHeight() {
  if (heightFloat < 0) 
  {
    height.hSign = 1; 
  }
  else
  {
    height.hSign = 0;
  }
  height.hInt = fabs(heightFloat);
  height.hDec = (fabs(heightFloat) - height.hInt) * 10;
  // Writing number to LCD
  if (height.hSign == 1) // Negative number
  {
    if(height.hInt >= 10)
    {
      lcd_gotoxy(10, 1);
      lcd_putc('-');
      itoa(height.hInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 1);
      itoa(height.hDec, print, 10);
      lcd_puts(print);
    }
    else
    {
      lcd_gotoxy(10, 1);
      lcd_puts(" -");
      itoa(height.hInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 1);
      itoa(height.hDec, print, 10);
      lcd_puts(print);
    }
  }
  else // Positive number
  {
      if(height.hInt >= 10)
    {
      lcd_gotoxy(10, 1);
      lcd_putc(' ');
      itoa(height.hInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 1);
      itoa(height.hDec, print, 10);
      lcd_puts(print);
    }
    else
    {
      lcd_gotoxy(10, 1);
      lcd_puts("  ");
      itoa(height.hInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(14, 1);
      itoa(height.hDec, print, 10);
      lcd_puts(print);
    }
  }
  updateHeight--; // Update completed 
}

/*
 * Function: Main function where the program execution begins
 * Purpose: 
 * Returns:  none
 */
int main(void)
{
  // -- Local variables -- //
  // Custom characters
  uint8_t leftBar[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0};
  uint8_t leftCenterBar[8] = {0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x0};
  uint8_t rightCenterBar[8] = {0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x0};
  uint8_t rightBar[8] = {0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0};
  uint8_t center[8] = {0xa,0x0,0x0,0x0,0x0,0x0,0xa,0x0};

  // Configure  Timers/Counters
  TIM0_ovf_1ms();   // Set prescaler to 1 ms
  TIM2_ovf_1ms();   // Set prescaler to 1 ms

  // Interrupts must be enabled, bacause of `uart_puts()`
  sei();

  // Initialize USART to asynchronous, 8-N-1, 115200 Bd
  uart_init(UART_BAUD_SELECT(115200, F_CPU));
  uart_puts("\r\nUART at 115200 Bd.\r\n");

  // Initialize display
  lcd_init(LCD_DISP_ON);
  lcd_custom_char(0, leftBar);
  lcd_custom_char(1, leftCenterBar);
  lcd_custom_char(2, rightCenterBar);
  lcd_custom_char(3, rightBar);
  lcd_custom_char(4, center);
  lcd_clrscr();
  lcd_home();

  // Main screen
  lcd_puts("I   ");
  lcd_putc(4);
  lcd_puts("   I -00.0");
  lcd_putc(0xDF);
  lcd_gotoxy(0,1);
  lcd_puts("          -00.0m");
  
  angleFloat = -96;
  heightFloat = -100;
  TIM0_ovf_enable();
  TIM2_ovf_enable();

  // Infinite loop
  while (1)
  {
    if(updateAngle == 1) updAngle();
    if(updateHeight == 1) updHeight();
  }
  return 0;
}

// -- Interrupt service routines --
/*
 * Function: Timer/Counter0 overflow interrupt
 * Purpose:  Simulate angle update.
*/
ISR(TIMER0_OVF_vect)
{
  static uint16_t ovfs = 0;
  TCNT0 = 6;    // accuration for 1ms
  ovfs++;
  if(ovfs >= 53)
  {
    if (angleFloat >= 95.9) {
      angleFloat = -95.9;
    }
    else {
      angleFloat += 0.1;
    }
  updateAngle++;
  ovfs = 0;
  }
}
/*
 * Function: Timer/Counter2 overflow interrupt
 * Purpose:  Simulate height update.
*/
ISR(TIMER2_OVF_vect)
{
  static uint16_t ovfs = 0;
  TCNT0 = 6;    // accuration for 1ms
  ovfs++;
  if(ovfs >= 47)
  {
    if (heightFloat >= 99.9) {
      heightFloat = -99.9;
    }
    else {
      heightFloat += 0.1;
    }
  updateHeight++;
  ovfs = 0;
  }
}