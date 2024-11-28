/*
  Simulating angle and height data update to 16x2 LCD.
  Created by Jakub Kovac
  24. 11. 2024
*/

// -- Includes --
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
// #include <uart.h>           // Peter Fleury's UART library (for debug messages if needed)
#include "screen.h"         // Custom C library.

// -- Global variables -- 
// "Flags"
volatile uint8_t updateAngleFlag = 0;
volatile uint8_t updateHeightFlag = 0;

// Measured data
float angle = 0;
float height = 0;

/*
 * Function: Main function where the program execution begins
 * Purpose: 
 * Returns:  none
 */
int main(void)
{
  // -- Configure  Timers/Counters --
  // Setting prescalers
  TIM0_ovf_1ms();   // 1 ms
  TIM2_ovf_1ms();   // 1 ms
  sei(); // Enabling interrupts, also needed for 'uart_puts()'

  // -- Initialize USART to asynchronous, 8-N-1, 115200 Bd --
  // For debugging
  // uart_init(UART_BAUD_SELECT(115200, F_CPU));
  // uart_puts("\r\nUART running at 115200 Bd.\r\n");

  lcdInit(); // -- Initialize display --
  
  // -- Main screen --
  lcd_puts("I   ");
  lcd_putc(4);
  lcd_puts("   I -00.0");
  lcd_putc(0xDF);
  lcd_gotoxy(0,1);
  lcd_puts("          -00.0m");
  
  // -- Filling variables with initial values and enabling timers' overflowing --
  angle = -96;
  height = -100;
  TIM0_ovf_enable();
  TIM2_ovf_enable();

  // -- Infinite loop --
  while (1) // Simulating screen update
  {
    if(updateAngleFlag == 1)
    { 
      updateAngle(angle);
      updateAngleFlag--;
    }
    if(updateHeightFlag == 1) 
    {
      updateHeight(height);
      updateHeightFlag--;
    }
  }

  return 0;
}

//-- Interrupt service routines --
/*
 * Function: Timer/Counter0 overflow interrupt
 * Purpose:  Simulate angle update every 50 ms.
*/
ISR(TIMER0_OVF_vect)
{
  static uint8_t ovfs = 0;
  TCNT0 = 6;    // accuration for 1ms
  ovfs++;
  if(ovfs >= 50)
  {
    if (angle >= 95.9) {
      angle = -95.9;
    }
    else {
      angle += 0.1;
    }
  updateAngleFlag++;
  ovfs = 0;
  }
}
/*
 * Function: Timer/Counter2 overflow interrupt
 * Purpose:  Simulate height update every 50 ms.
*/
ISR(TIMER2_OVF_vect)
{
  static uint8_t ovfs = 0;
  TCNT0 = 6; // accuration for 1ms
  ovfs++;
  if(ovfs >= 50)
  {
    if (height >= 99.9) {
      height = -99.9;
    }
    else {
      height += 0.1;
    }
  updateHeightFlag++;
  ovfs = 0;
  }
}