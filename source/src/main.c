/*
  BPC-DE2 Projekt
*/

// -- Includes -- //
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "twi.h"
#include <lcd.h>            // Peter Fleury's LCD library
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include <math.h>           // C library fabs()
#include "mpu6050.h"
#include "laser.h"          // Laser library
#include <util/delay.h> 


#define MPU6050_ADDRESS 0x68         // I2C address of MPU6050
#define calibrate_time 1000
#define LASER_PIN PB2
#define BUTTON_PIN PD2


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

  if (angle.aSign == 1) 
  {
    if(angle.aInt >= 10)
    {
      lcd_gotoxy(10, 0);
      lcd_putc('-');
      itoa(angle.aInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(13, 0);
      lcd_putc(',');
      itoa(angle.aDec, print, 10);
      lcd_puts(print);
    }
    else
    {
      lcd_gotoxy(10, 0);
      lcd_puts(" -");
      itoa(angle.aInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(13, 0);
      lcd_putc(',');
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
      lcd_gotoxy(13, 0);
      lcd_putc(',');
      itoa(angle.aDec, print, 10);
      lcd_puts(print);
    }
    else
    {
      lcd_gotoxy(10, 0);
      lcd_puts("  ");
      itoa(angle.aInt, print, 10);
      lcd_puts(print);
      lcd_gotoxy(13, 0);
      lcd_putc(',');
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

  lcd_gotoxy(0, 0);
  lcd_puts("I   ");
  lcd_putc(4);
  lcd_puts("   I");
  if (angle.aSign == 0)
  {
    lcd_gotoxy(4+barPosition, 0);

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
  char string[8];

  // -- Local variables -- //
  // Custom characters
  uint8_t leftBar[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0};
  uint8_t leftCenterBar[8] = {0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x0};
  uint8_t rightCenterBar[8] = {0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x0};
  uint8_t rightBar[8] = {0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0};
  uint8_t center[8] = {0xa,0x0,0x0,0x0,0x0,0x0,0xa,0x0};


  // Initialize Laser and Button
  GPIO_mode_output(&DDRB, LASER_PIN);        // Configure laser pin as output
  GPIO_write_low(&PORTB, LASER_PIN);         // Set laser pin to LOW (laser off)
  GPIO_mode_input_pullup(&DDRD, BUTTON_PIN); // Configure button pin as input with pull-up resistor

  // Initialize USART to asynchronous, 8-N-1, 115200 Bd
  uart_init(UART_BAUD_SELECT(115200, F_CPU));
  uart_puts("\r\nUART at 115200 Bd.\r\n");

  sei();

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
  
  // Initialize TWI
  twi_init(); 

  // // Test connection to MPU6050
  // if (twi_test_address(MPU6050_ADDRESS) != 0) {
  //     uart_puts("[ERROR] MPU6050 device not detected\r\n");
  //     while (1);
  // } 

  // Initialize MPU6050
  mpu6050_init();

  // Calibrate MPU6050
  lcd_clrscr();
  lcd_home();

  // Wait for calibration to be completed screen
  lcd_puts("  Calibration, ");
  lcd_gotoxy(0,1);
  lcd_puts("  please wait.");

  mpu6050_calibrate();
  lcd_clrscr();
  lcd_home();
  lcd_puts("      Done.     ");
  _delay_ms(1000);


  // Main screen
  lcd_clrscr();
  lcd_home();
  lcd_puts("I   ");
  lcd_putc(4);
  lcd_puts("   I -00,0");
  lcd_putc(0xDF);
  // lcd_gotoxy(0,1);
  // lcd_puts("          -00,0m");
  // uart_puts("Main scr.\r\n");

  lcd_gotoxy(0,1);
  lcd_puts("Laser OFF");

  
  
  TIM1_ovf_4ms();   
  TIM1_ovf_enable();

  // Infinite loop
  while (1)
  {
    handle_button();
    _delay_ms(50); 
    handle_laser_timeout();
    if(updateAngle == 1) updAngle();

    // uart_puts(" AccX: ");
    // dtostrf(accel_values[0], 6, 2, string);
    // uart_puts(string);
    // uart_puts(" | ");
    // uart_puts(" AccY: ");
    // dtostrf(accel_values[1], 6, 2, string);
    // uart_puts(string);
    // uart_puts(" | ");
    // uart_puts(" AccZ: ");
    // dtostrf(accel_values[2], 6, 2, string);
    // uart_puts(string);
    // uart_puts(" | ");
    // uart_puts("\r\n");
    
  }
  return 0;
}

// -- Interrupt service routines -------------------------------------
/*
 * Function: TIMER1_OVF_vect
 * Purpose:  Timer overflow interrupt to read data from MPU6050
 */
ISR(TIMER1_OVF_vect) {
    static uint8_t n_ovfs = 0;
    static uint16_t overflow_count = 0;
    TCNT1 = 1536;
    n_ovfs++;
    overflow_count++;

    mpu6050_read_data();  // Read new data from MPU6050
    angleFloat = calculate_angles();   // Update pitch and roll angles

    // Do this every 50 x 4 ms = 200 ms
    if (n_ovfs >= 50 && angleFloat < 90 && angleFloat > -90) {
        updateAngle = 1;   //
        n_ovfs = 0;        // Reset overflow counter
    }

    // For laser 
    if (overflow_count >= 25) {
        overflow_count = 0;
        if (laser_on) {
            laser_timer++; 
        }
    }
}
