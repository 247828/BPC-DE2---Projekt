/*
  BPC-DE2 Projekt
*/

// -- Includes -- //
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include <math.h>           // C library fabs()

// -- Global variables -- //
// "Flags"
volatile uint8_t updateAngle = 0;
volatile uint8_t updateHeight = 0;

volatile uint8_t calibScreen = 0;
volatile uint8_t mainScreen = 0;

char print[3]; // itoa

float angleFloat = 0;

struct Angle_structure {
    int8_t aInt;    // Integer part
    uint8_t aDec;   // Decimal part
    uint8_t aSign; // Sign (0 positive or 1 negative)
} angle;
/*
struct Height_structure {
    int8_t hInt;    // Integer part
    uint8_t hDec;   // Decimal part
    uint8_t hSign; // Sign (0 positive or 1 negative)
} height;
*/

// -- Function definitions --
/*
 * Function: updAngle
 * Purpose:  Update the screen with new angle information and graphical represantation.
 * Returns:  none
 */
void updAngle() {
  static int8_t barPosition = 0;
  // float to dec
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
  /*
  uart_puts("New angle: ");
  itoa(angle.aSign, print, 10);
  uart_puts(print);
  uart_putc(' ');
  itoa(angle.aInt, print, 10);
  uart_puts(print);
  uart_putc(',');
  itoa(angle.aDec, print, 10);
  uart_puts(print);
  uart_puts("\r\n");
  */
  if (angle.aSign == 1) 
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
  else 
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
  // Bar position
  if(angle.aInt >= 6)
  { 
    barPosition = (angle.aInt-6)/30 + 1;
  }
  else
  {
    barPosition = 0;
  }
  /*
  uart_puts("Bar position: ");
  itoa(barPosition, print, 10);
  uart_puts(print);
  uart_puts("\r\n");
  */
  lcd_gotoxy(1, 0);
  lcd_puts("   ");
  lcd_putc(4);
  lcd_puts("   ");
  if (angle.aSign == 0)
  {
    lcd_gotoxy(4+barPosition, 0);
    /*
    itoa(4+barPosition, print, 10);
    uart_puts(print);
    uart_puts("\r\n");
    */

  }
  else
  {
    lcd_gotoxy(4-barPosition, 0);
    /*
    itoa(4-barPosition, print, 10);
    uart_puts(print);
    uart_puts("\r\n");
    */
  }
  //
  if(angle.aInt < 6 && angle.aInt > -6) lcd_putc('I');
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
  updateAngle--;
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
  TIM0_ovf_1ms();      // Set prescaler to 1 sec and enable overflow interrupt
  TIM1_ovf_1sec();      // Set prescaler to 1 sec and enable overflow interrupt

  // Enable overflow interrupt
  //TIM1_ovf_enable();
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
  uart_puts("LCD initialized.\r\n");
  /*
  // Welcome screen
  lcd_puts("PROJEKT BPC-DE2.");
  lcd_gotoxy(0,1);
  lcd_puts("    ArNiHoSo    ");
  uart_puts("Welcome scr.\r\n");
  while (calibScreen == 0)
  {

  }
  calibScreen--;
  lcd_clrscr();
  lcd_home();

  // Wait for calibration to be completed screen
  lcd_puts("POCKEJTE PROSIM,");
  lcd_gotoxy(0,1);
  lcd_puts("   kalibrace.   ");
  uart_puts("Calib. scr.\r\n");
  while (mainScreen == 0)
  {

  }
  mainScreen--;
  lcd_clrscr();
  lcd_home();
  */
  // Main screen
  lcd_puts("I   ");
  lcd_putc(4);
  lcd_puts("   I -00,0");
  lcd_putc(0xDF);
  lcd_gotoxy(0,1);
  lcd_puts("          -00,0m");
  uart_puts("Main scr.\r\n");
  
  angleFloat = -96;
  TIM0_ovf_enable();
  uart_puts("Angle set 90d and TCNT0 enabled.\r\n");
  // Infinite loop
  while (1)
  {
    if(updateAngle == 1) updAngle();
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
  if(ovfs >= 50)
  {
    if (angleFloat >= 95.9) {
      angleFloat = -95.9;
    }
    else {
      angleFloat += 0.1;
    }
  //uart_puts("TCNT0: 100 ms has passed.\r\n");
  updateAngle++;
  ovfs = 0;
  }
}
// -- Interrupt service routines --
/*
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Simulate loading.
 */
ISR(TIMER1_OVF_vect)
{
  static uint16_t ovfs = 0;
  TCNT1 = 3036;
  ovfs++;
  uart_puts("TCNT1: 1 s has passed.\r\n");
  if (ovfs == 1) 
  {
    calibScreen++;
  }
  if (ovfs == 2)
  {
    mainScreen++;
    TIM1_ovf_disable();
    ovfs = 0;
  }
}