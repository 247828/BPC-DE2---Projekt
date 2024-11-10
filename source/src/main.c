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
#include <math.h> // C library abs()

// -- Global variables -- //
// "Flags"
volatile uint8_t updateAngle = 0;
volatile uint8_t updateHeight = 0;

volatile uint8_t calibScreen = 0;
volatile uint8_t mainScreen = 0;

char print[3]; // itoa

struct Angle_structure {
    int8_t aInt;    // Integer part
    uint8_t aDec;   // Decimal part
    uint8_t negDec; // negative decimal part from -0.1 to -0.9
} angle;
/*
struct Height_structure {
    int8_t hInt;    // Integer part
    uint8_t hDec;   // Decimal part
} height;
*/

// -- Function definitions --
/*
 * Function: updAngle
 * Purpose:  Update the screen with new angle information and graphical represantation.
 * Returns:  none
 */
void updAngle(int8_t aInt, uint8_t aDec, uint8_t signDec) {
  static int8_t barPosition = 0;
  static uint8_t absBarPoisition = 0;
  uart_puts("New angle: ");
      itoa(abs(aInt), print, 10);
      uart_puts(print);
      uart_puts("\r\n");
      //uart_puts(decString);
      //uart_puts("\r\n");
      
      if(aInt < 0) {
        if(aInt > -10) {
          lcd_gotoxy(10, 0);
          lcd_puts(" -");
          lcd_puts(print);
          //lcd_putc(',');
          //lcd_putc(decString[0]);
        }
        else {
          lcd_gotoxy(10, 0);
          lcd_putc('-');
          lcd_puts(print);
          //lcd_putc(',');
          //lcd_putc(decString[0]);
        }
      }
      else {
        /*if(angle.negDec == 1) 
        {
          lcd_gotoxy(10, 0);
          lcd_puts(" -0,");
          lcd_putc(decString[0]);
        }*/
        if(aInt < 10) {
          lcd_gotoxy(10, 0);
          lcd_puts("  ");
          lcd_puts(print);
          //lcd_putc(',');
          //lcd_putc(decString[0]);
        }
        else {
          lcd_gotoxy(10, 0);
          lcd_putc(' ');
          lcd_puts(print);
          //lcd_putc(',');
          //lcd_putc(decString[0]);
        }
      }
      // Bar position
      if(aInt <= -6) barPosition = (aInt+6)/30 - 1;
      else if(aInt >= 6) barPosition = (aInt-6)/30 + 1;
      else barPosition = 0;
      absBarPoisition = abs(barPosition);
      uart_puts("Bar position: ");
      itoa(barPosition, print, 10);
      uart_puts(print);
      uart_puts("\r\n");

      lcd_gotoxy(1, 0);
      lcd_puts("   ");
      lcd_putc(4);
      lcd_puts("   ");
      lcd_gotoxy(4+barPosition, 0);
      
      if(aInt < 6 && aInt > -6) lcd_putc('I');
      else if(aInt <= (-6)-(absBarPoisition-1)*30 && aInt > (-12)-(absBarPoisition-1)*30 || aInt >= 30+(absBarPoisition-1)*30 && aInt < 36+(absBarPoisition-1)*30) lcd_putc(3);
      else if(aInt <= (-12)-(absBarPoisition-1)*30 && aInt > (-18)-(absBarPoisition-1)*30 || aInt >= 24+(absBarPoisition-1)*30 && aInt < 30+(absBarPoisition-1)*30) lcd_putc(2);
      else if(aInt <= (-18)-(absBarPoisition-1)*30 && aInt > (-24)-(absBarPoisition-1)*30 || aInt >= 18+(absBarPoisition-1)*30 && aInt < 24+(absBarPoisition-1)*30) lcd_putc('|');
      else if(aInt <= (-24)-(absBarPoisition-1)*30 && aInt > (-30)-(absBarPoisition-1)*30 || aInt >= 12+(absBarPoisition-1)*30 && aInt < 18+(absBarPoisition-1)*30) lcd_putc(1);
      else if(aInt <= (-30)-(absBarPoisition-1)*30 && aInt > (-36)-(absBarPoisition-1)*30 || aInt >= 6+(absBarPoisition-1)*30 && aInt < 12+(absBarPoisition-1)*30) lcd_putc(0);

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

  uint8_t leftBar[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0};
  uint8_t leftCenterBar[8] = {0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x0};
  uint8_t rightCenterBar[8] = {0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x0};
  uint8_t rightBar[8] = {0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0};
  uint8_t center[8] = {0xa,0x0,0x0,0x0,0x0,0x0,0xa,0x0};


  // Configure  Timers/Counters
  TIM0_ovf_1ms();      // Set prescaler to 1 sec and enable overflow interrupt
  TIM1_ovf_1sec();      // Set prescaler to 1 sec and enable overflow interrupt

  // Enable overflow interrupt
  TIM1_ovf_enable();
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
  // Main screen
  lcd_puts("I   ");
  lcd_putc(4);
  lcd_puts("   I -00,0");
  lcd_putc(0xDF);
  lcd_gotoxy(0,1);
  lcd_puts("          -00,0m");
  uart_puts("Main scr.\r\n");
  angle.aInt = 95;
  TIM0_ovf_enable();
  uart_puts("Angle set 90d and TCNT0 enabled.\r\n");
  // Infinite loop
  while (1)
  {
    if(updateAngle == 1) updAngle(angle.aInt, angle.aDec, angle.negDec);
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
    if (angle.aInt >= 95) {
      angle.aInt = -95;
    }
    else {
      angle.aInt++;
    }
  uart_puts("TCNT0: 50 ms has passed.\r\n");
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