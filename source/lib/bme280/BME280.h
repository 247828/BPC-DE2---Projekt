#ifndef BME280_H
#define BME280_H

/**
 * @name BME280 I2C Address
 * @brief I2C address for BME280 sensor.
 * @{
 */
#define BME_slave 0x76 /**< @brief I2C address for BME280 */
/** @} */

/**
 * @name Register Addresses
 * @brief Register addresses for accessing BME280 data and configuration.
 * @{
 */
#define BME_PRESS_MEM 0xF7 /**< @brief Register address for pressure data */
#define BME_TEMP_MEM 0xFA  /**< @brief Register address for temperature data */
#define BME_CONFIG 0xF5    /**< @brief Register address for sensor configuration */
#define BME_ctrlmeas 0xF4  /**< @brief Register address for control measurement settings */
#define BME_Ctrlhum 0xF2   /**< @brief Register address for humidity control settings */
#define BME_status 0xF3    /**< @brief Register address for sensor status */

// Pins for BME280 on TWI1 (using A4 and A5)
#define BME280_SDA_PIN 4
#define BME280_SCL_PIN 5
/** @} */

/**
 * @name Constants
 * @brief General constants for program configuration.
 * @{
 */
#define avr_time 50     /**< @brief Time interval for averaging sensor data (in ticks) */
#define BUTTON2_PIN PD3    /**< @brief Pin for button input */
/** @} */

/**
 * @name Function Prototypes
 * @brief Function declarations for initializing, reading, and processing BME280 data.
 * @{
 */

/**
 * @brief Load temperature calibration data from BME280 sensor.
 * @return void
 */
void load_temp_calibration_data(void);

/**
 * @brief Load pressure calibration data from BME280 sensor.
 * @return void
 */
void load_press_calibration_data(void);

/**
 * @brief Configure BME280 sensor with default settings.
 * @return void
 */
void configure_bme280(void);

/**
 * @brief Read raw temperature and pressure data from BME280.
 * @return void
 */
void bme280_read_data(void);

/**
 * @brief Calculate height based on pressure data.
 * @return void
 */
void calculate_heist(void);

/**
 * @brief Handle button press and update reference pressure or state.
 * @return void
 */
void handle_button_bme(void);

/**
 * @brief Print calculated height, temperature, and pressure via UART.
 * @return void
 */
float height_print(void);
/** @} */

#endif // BME280_H