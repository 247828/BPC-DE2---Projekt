#include "BME280.h"
#include <twi.h> 
#include <math.h>
#include <gpio.h> 
#include <util/delay.h>
#include <uart.h>

// Global variables for calibration constants
uint16_t dig_T1, dig_P1;
int16_t dig_T2, dig_T3;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
int32_t t_fine;  // Variable for fine temperature adjustment

// Global variables for sensor data and calculations
volatile uint8_t last_button_state_bme = 1;
volatile float pressure1 = 0.0;        // Reference pressure when button is pressed
volatile float pressure = 0.0;         // Current pressure reading
volatile float temperature = 0.0;      // Current temperature reading
volatile float height_bme = 0.0;           // Calculated height difference
volatile uint8_t flag_update_uart = 0; // Flag to indicate UART update
volatile uint8_t bme_values[3];        // Array to store raw sensor data
volatile uint32_t raw_temp = 0, raw_press = 0; // Raw temperature and pressure values

// Variables for assembling raw data
uint32_t temp_high, temp_mid, temp_low;
uint32_t press_high, press_mid, press_low;
int32_t bme_rep1, bme_rep2, bme_rep3;

uint8_t debounce_time = 50;

// Variables for averaging calculations
volatile uint8_t last_button_state = 1;
volatile float pressure_sum = 0.0;
volatile float temperature_sum = 0.0;
volatile uint8_t pressure_count = 0;
volatile uint8_t temperature_count = 0;
volatile float pressure_avg = 0.0;
volatile float temperature_avg = 0.0;

// BME280 sensor configuration
void configure_bme280(void) {
    twi_start();  // Start I2C communication
    twi_write((BME_slave << 1) | TWI_WRITE); // Address BME280 with write operation
    twi_write(0xF4); // Write to control measurement register
    twi_write(0b10110111); // Set oversampling and mode settings
    twi_stop(); // Stop I2C communication

    uint8_t config_value = (0 << 5) | (5 << 2); // Filter and standby time configuration
    twi_start(); // Start I2C communication
    twi_write((BME_slave << 1) | TWI_WRITE); // Address BME280 with write operation
    twi_write(BME_CONFIG); // Write to configuration register
    twi_write(config_value); // Write configuration settings
    twi_stop(); // Stop I2C communication
}

void read_bme280(void) {
    twi_set_pins(BME280_SDA_PIN, BME280_SCL_PIN); // Přepnout na piny pro BME280
    bme280_read_data();                          // Číst data z BME280
}


// Load temperature calibration constants from BME280
void load_temp_calibration_data(void) {
    uint8_t calib_data[6]; // Array to hold calibration data
    twi_readfrom_mem_into(BME_slave, 0x88, calib_data, 6); // Read 6 bytes starting from register 0x88

    // Combine bytes to form calibration constants
    dig_T1 = (calib_data[1] << 8) | calib_data[0];
    dig_T2 = (calib_data[3] << 8) | calib_data[2];
    dig_T3 = (calib_data[5] << 8) | calib_data[4];
}

// Load pressure calibration constants from BME280
void load_press_calibration_data(void) {
    uint8_t calib_data[18]; // Array to hold calibration data
    twi_readfrom_mem_into(BME_slave, 0x8E, calib_data, 18); // Read 18 bytes starting from register 0x8E

    // Combine bytes to form calibration constants
    dig_P1 = (calib_data[1] << 8) | calib_data[0];
    dig_P2 = (calib_data[3] << 8) | calib_data[2];
    dig_P3 = (calib_data[5] << 8) | calib_data[4];
    dig_P4 = (calib_data[7] << 8) | calib_data[6];
    dig_P5 = (calib_data[9] << 8) | calib_data[8];
    dig_P6 = (calib_data[11] << 8) | calib_data[10];
    dig_P7 = (calib_data[13] << 8) | calib_data[12];
    dig_P8 = (calib_data[15] << 8) | calib_data[14];
    dig_P9 = (calib_data[17] << 8) | calib_data[16];
}

// Reading data from the sensor

// Temperature calculation using calibration data
float calculate_temperature(int32_t raw_tempc) {
    int32_t var1, var2;

    // Temperature compensation formula from BME280 datasheet
    var1 = ((((raw_tempc >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((raw_tempc >> 4) - ((int32_t)dig_T1)) * ((raw_tempc >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2; // Fine temperature value for pressure compensation

    float temp = ((t_fine * 5 + 128) >> 8) / 100.0; // Actual temperature in °C

    // Accumulate temperature values and calculate average
    temperature_sum += temp;
    temperature_count++;

    if (temperature_count >= avr_time) { // Every 'avr_time' samples, compute average
        temperature_avg = temperature_sum / temperature_count;
        temperature_sum = 0.0;   // Reset accumulator
        temperature_count = 0;  // Reset counter
    }

    return temperature_avg; // Return average temperature
}

// Pressure calculation using calibration data
float calculate_pressure(int32_t raw_pressc) {
    int64_t var1, var2, p;

    // Pressure compensation formula from BME280 datasheet
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // Prevent division by zero
    }

    p = 1048576 - raw_pressc;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    float pressure = (float)p / 256.0 / 100.0; // Actual pressure in hPa

    // Accumulate pressure values and calculate average
    pressure_sum += pressure;
    pressure_count++;

    if (pressure_count >= avr_time) { // Every 'avr_time' samples, compute average
        pressure_avg = pressure_sum / pressure_count;
        pressure_sum = 0.0;    // Reset accumulator
        pressure_count = 0;   // Reset counter
    }

    return pressure_avg; // Return average pressure
}

// Read data from BME280 sensor
void bme280_read_data(void) {
    // Reading raw temperature data
    twi_readfrom_mem_into(BME_slave, BME_TEMP_MEM, bme_values, 3);
    temp_high = (uint32_t)bme_values[0] << 12;
    temp_mid = (uint32_t)bme_values[1] << 4;
    temp_low = (uint32_t)(bme_values[2] & 0xF0) >> 4;

    raw_temp = temp_high | temp_mid | temp_low; // Combine bytes to form raw temperature

    bme_rep1 = bme_values[0];
    bme_rep2 = bme_values[1];
    bme_rep3 = bme_values[2];

    temperature = calculate_temperature(raw_temp); // Calculate temperature

    // Reading raw pressure data
    twi_readfrom_mem_into(BME_slave, BME_PRESS_MEM, bme_values, 3); 
    press_high = (uint32_t)bme_values[0] << 12;
    press_mid = (uint32_t)bme_values[1] << 4;
    press_low = (uint32_t)(bme_values[2] & 0xF0) >> 4;

    raw_press = press_high | press_mid | press_low; // Combine bytes to form raw pressure
    pressure = calculate_pressure(raw_press);      // Calculate pressure
}

// Calculate height difference based on pressure readings
float calculate_height_difference(float pressure1, float pressure2, float temperature) {
    float R = 8.314;
    float T0 = 273.15;
    float g = 9.80665;
    float M = 0.0289644;

    float temp_kelvin = temperature + T0;
    // Calculate height difference using the barometric formula
    float height = ((R*temp_kelvin)/(g*M)) * log(pressure1/pressure2); 
    
    return height;  // Return height in meters
}

// Handle button press events
void handle_button_bme(void) {
    uint8_t current_state = GPIO_read(&PIND, BUTTON2_PIN); // Read the current button state

    // Detect button state change
    if (current_state != last_button_state) {
        _delay_ms(debounce_time); // Wait for debounce time

        // Check if the button is still in the same state
        if (GPIO_read(&PIND, BUTTON2_PIN) == current_state) {
            // Save the new state
            last_button_state = current_state;

            // If the button was pressed (falling edge detection)
            if (current_state == 0) {
                uart_puts("Button ON\r\n"); // Debug message

                // Save the current pressure as reference
                pressure1 = pressure;

                // Reset height to 0 (optional)
                height_bme = 0.0; 
            }
        }
    }
}

// Calculate and update height difference
float height_print(void){
    height_bme = calculate_height_difference(pressure1, pressure, temperature); // Update height calculation
    return height_bme;
}

