#include "laser.h"
#include <gpio.h>  // GPIO library for pin manipulation
#include <uart.h>  
#include <lcd.h>            // Peter Fleury's LCD library


#define LASER_PIN PB2
#define BUTTON_PIN PD2

// Global variables
volatile uint8_t laser_on = 0;       // Laser state (0 = OFF, 1 = ON)
volatile uint8_t laser_timer = 0;    // Timer for laser timeout
volatile uint8_t button_state = 0;   // Current button state
volatile uint8_t last_button_state = 1; // Previous button state (active low)

/*
 * Function: handle_laser_timeout
 * Purpose:  Automatically turn off the laser after timeout.
 * Returns:  none
 */
void handle_laser_timeout(void) {
    if (laser_on && laser_timer >= LASER_TIMEOUT) {
        laser_on = 0;                           // Turn off laser
        GPIO_write_low(&PORTB, LASER_PIN);      // Set laser pin LOW

        // Debug and LCD message
        uart_puts("Laser OFF (timeout)\r\n");
        lcd_gotoxy(0, 1);
        lcd_puts("Laser OFF");  // Update LCD message
    }
}

/*
 * Function: handle_button
 * Purpose:  Check the button state and toggle the laser on/off.
 * Returns:  none
 */
void handle_button(void) {
    uint8_t current_state = GPIO_read(&PIND, BUTTON_PIN); // Read the current state of the button

    // Detect button press (falling edge detection)
    if (current_state == 0 && last_button_state == 1) {
        // Toggle the laser state
        laser_on = !laser_on;

        if (laser_on) {
            GPIO_write_high(&PORTB, LASER_PIN);  // Turn laser ON
            uart_puts("Laser ON\r\n");          // Debug message
            lcd_gotoxy(0, 1);
            lcd_puts("Laser ON ");
            laser_timer = 0;                    // Reset the timeout timer
        } else {
            GPIO_write_low(&PORTB, LASER_PIN);  // Turn laser OFF
            uart_puts("Laser OFF\r\n");         // Debug message
            lcd_gotoxy(0, 1);
            lcd_puts("Laser OFF");
        }
    }

    // Save the current state for the next comparison
    last_button_state = current_state;
}
