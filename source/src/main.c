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
#include "BME280.h"
#include <BME280.c>


#define MPU6050_ADDRESS 0x68         // I2C address of MPU6050
#define calibrate_time 1000
#define LASER_PIN PB2
#define BUTTON1_PIN PD2
#define BME_slave 0x76

// Pins for MPU6050 on TWI0
#define MPU6050_SDA_PIN 19
#define MPU6050_SCL_PIN 18

// Pins for BME280 on TWI1 (using A4 and A5)
#define BME280_SDA_PIN 4
#define BME280_SCL_PIN 5

// -- Global variables -- //
// "Flags"
volatile uint8_t updateAngle = 0;
volatile uint8_t updateHeight = 0;
volatile uint8_t calibScreen = 0;
volatile uint8_t mainScreen = 0;

char print[3]; // itoa

float angleFloat = 0;
float heightFloat = 0;

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
  

  // -- Local variables -- //
  // Custom characters
  uint8_t leftBar[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0};
  uint8_t leftCenterBar[8] = {0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x0};
  uint8_t rightCenterBar[8] = {0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x0};
  uint8_t rightBar[8] = {0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0};
  uint8_t center[8] = {0xa,0x0,0x0,0x0,0x0,0x0,0xa,0x0};


  // Local variables
  char height_string[10]; // Buffer for height UART output
  char angle_string[10];
  // char string[8];

  // Initialize Laser and Button
  GPIO_mode_output(&DDRB, LASER_PIN);        // Configure laser pin as output
  GPIO_write_low(&PORTB, LASER_PIN);         // Set laser pin to LOW (laser off)
  GPIO_mode_input_pullup(&DDRD, BUTTON1_PIN); // Configure button pin as input with pull-up resistor
  GPIO_mode_input_pullup(&DDRD, BUTTON2_PIN); // Configure BME button pin as input with pull-up resistor

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

    // Initialize MPU6050
  twi_set_pins(MPU6050_SDA_PIN, MPU6050_SCL_PIN);
  mpu6050_init();

  if (twi_test_address(MPU6050_ADDRESS) != 0) {
      uart_puts("[ERROR] MPU6050 device not detected\r\n");
      while (1);
  } 

  // Initialize BME280
  twi_set_pins(BME280_SDA_PIN, BME280_SCL_PIN);
  configure_bme280();
  load_temp_calibration_data();
  load_press_calibration_data();

  // Test connection to BME280
  if (twi_test_address(BME_slave) != 0) {
      uart_puts("[ERROR] BME280 device not detected\r\n");
      while (1);
  } 

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

  TIM2_ovf_16ms();
  TIM2_ovf_enable();
  
  // Infinite loop
  while (1)
  {
    handle_button_laser();
    handle_button_bme();     
    handle_laser_timeout();

    if (updateHeight == 1){
    // Convert height to string and send to UART
      dtostrf(heightFloat, 6, 2, height_string);
      uart_puts("Height: ");
      uart_puts(height_string);
      updateHeight = 0;
      uart_puts("\r\n");
    }

    if(updateAngle == 1) {   
      updAngle();
    // Convert angle to string and send to UART
      dtostrf(angleFloat, 6, 1, angle_string); 
      uart_puts("Angle: ");
      uart_puts(angle_string);
      uart_puts("\r\n");

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

        updateAngle = 0;
      }
    
    }
  return 0;
} 


// -- Interrupt service routines -------------------------------------
/*
 * Function: TIMER1_OVF_vect
 * Purpose:  Timer overflow interrupt to read data from MPU6050
 */
ISR(TIMER1_OVF_vect) {
    static uint16_t n_ovfs = 0;
    static uint8_t overflow_count = 0;
    TCNT1 = 1536;
    n_ovfs++;
    overflow_count++;
    read_mpu6050(); // Read new data from MPU6050
    angleFloat = calculate_angles();   // Update pitch and roll angles
    // Do this every 50 x 4 ms = 200 ms
    if (n_ovfs >= 50 && angleFloat < 90 && angleFloat > -90) {
        updateAngle = 1;   //
        n_ovfs = 0;        // Reset overflow counter
    }

    // 4 x 10 x 250(LASER_TIMEOUT in laser.h)  = 10 sec
    if (overflow_count >= 10) {
        overflow_count = 0;
        if (laser_on) {
            laser_timer++; 
        }
    }
}

ISR(TIMER2_OVF_vect) {
    static uint8_t overflow_bme = 0;
    overflow_bme++;
    read_bme280();  // Read BME280 data 
    if (overflow_bme >= avr_time) {
        heightFloat = height_print();
        updateHeight = 1;
        overflow_bme = 0;
    }
}
