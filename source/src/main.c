/*
  BPC-DE2 Project - variant with screen routines in and uart output in main.c file
  * @author Artur Nizamutdinov, Nikita Kolobov, Jan Bozejovsky, Jakub Kovac
  * 29. 11. 2024
*/

// -- Includes -- //
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "twi.h"            // I2C/TWI library
#include <lcd.h>            // Peter Fleury's LCD library
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include <math.h>           // C library fabs()
#include "mpu6050.h"        // Accelerometer and Gyroscope Sensor library
#include "laser.h"          // Laser library
#include <util/delay.h>     // for delay
#include "BME280.h"         // Humidity sensor library
#include <BME280.c>         // needed ?
#include "screen.h"      // Screen library for 16x2 LCD screen (!needed test!)

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
volatile uint8_t updateAngleFlag = 0;
volatile uint8_t updateHeightFlag = 0;
//
float angleFloat = 0;
float heightFloat = 0;

/*
 * Function: Main function where the program execution begins
 * Purpose: 
 * Returns:  none
 */
int main(void)
{
  // Initialize Laser and Button
  GPIO_mode_output(&DDRB, LASER_PIN);        // Configure laser pin as output
  GPIO_write_low(&PORTB, LASER_PIN);         // Set laser pin to LOW (laser off)
  GPIO_mode_input_pullup(&DDRD, BUTTON1_PIN); // Configure button pin as input with pull-up resistor
  GPIO_mode_input_pullup(&DDRD, BUTTON2_PIN); // Configure BME button pin as input with pull-up resistor

// Initialize USART to asynchronous, 8-N-1, 115200 Bd
  uart_init(UART_BAUD_SELECT(115200, F_CPU));
  // uart_puts("\r\nUART at 115200 Bd.\r\n");

  sei(); // Enable interrupts

  // Initialize display
  lcdInit();
  
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
  lcd_gotoxy(11,1);
  lcd_puts("00,0m");
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

    if (updateHeightFlag == 1){
      updateHeight(heightFloat);
      updateHeightFlag--;
    }

    if(updateAngleFlag == 1) {   
      updateAngle(angleFloat);
      updateAngleBar(angleFloat);
      updateAngleFlag--;
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
        updateAngleFlag++;   //
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
        updateHeightFlag++;
        overflow_bme = 0;
    }
}
