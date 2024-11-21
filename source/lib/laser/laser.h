#ifndef LASER_H
#define LASER_H

#include <stdint.h>  // Standard integer types

// Pin definitions
#define LASER_PIN PB2       // Pin for controlling the laser
#define BUTTON_PIN PD2      // Pin for the button
#define LASER_TIMEOUT 120   // Timeout in seconds (adjust as needed)

// External declarations of global variables
extern volatile uint8_t laser_on;          // Laser state (0 = OFF, 1 = ON)
extern volatile uint8_t laser_timer;       // Timer for laser timeout
extern volatile uint8_t button_state;      // Current button state
extern volatile uint8_t last_button_state; // Previous button state

// Function prototypes
void handle_button(void);               // Function to toggle laser state based on button press
void handle_laser_timeout(void);        // Function to turn off laser after timeout

#endif // LASER_H
