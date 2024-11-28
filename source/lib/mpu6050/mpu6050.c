// -- Includes -------------------------------------------------------
#include "mpu6050.h"
#include "twi.h"
#include <math.h> 


// -- Defines --------------------------------------------------------
#define ACCEL_XOUT_H 0x3B            // Register address for X-axis accelerometer data
#define GYRO_XOUT_H 0x43             // Register address for X-axis gyroscope data
#define calibrate_time 1000

// Pins for MPU6050 on TWI0
#define MPU6050_SDA_PIN 19
#define MPU6050_SCL_PIN 18

// -- Global variables -----------------------------------------------
volatile float accel_values[3] = {0};    // Accelerometer values (X, Y, Z)
volatile float gyro_values[3] = {0};     // Gyroscope values (X, Y, Z)
volatile float angle_pitch = 0;          // Pitch angle
volatile float angle_roll = 0;           // Roll angle
volatile long gyro_x_cal = 0, gyro_y_cal = 0, gyro_z_cal = 0;  // Gyroscope calibration



// -- Functions ------------------------------------------------------

/*
 * Function: mpu6050_init()
 * Purpose:  Initialize the MPU6050 sensor, configure it for +/-8g on the accelerometer
 *           and 500°/s on the gyroscope.
 * Returns:  none
 */
void mpu6050_init(void) {
    // Wake up MPU6050 by setting PWR_MGMT_1 register to 0x00
    twi_start();
    twi_write((MPU6050_ADDRESS << 1) | TWI_WRITE);
    twi_write(0x6B); // PWR_MGMT_1 register
    twi_write(0x00); // Wake up MPU6050
    twi_stop();

    // Configure accelerometer to +/-8g
    twi_start();
    twi_write((MPU6050_ADDRESS << 1) | TWI_WRITE);
    twi_write(0x1C); // ACCEL_CONFIG register
    twi_write(0x10); // Set sensitivity to +/-8g
    twi_stop();

    // Configure gyroscope to 500°/s
    twi_start();
    twi_write((MPU6050_ADDRESS << 1) | TWI_WRITE);
    twi_write(0x1B); // GYRO_CONFIG register
    twi_write(0x08); // Set sensitivity to 500°/s
    twi_stop();
}

void read_mpu6050(void) {
    twi_set_pins(MPU6050_SDA_PIN, MPU6050_SCL_PIN); // Change pins to MPU6050
    mpu6050_read_data();                           
}

/*
 * Function: mpu6050_read_data()
 * Purpose:  Reads raw data from the MPU6050 accelerometer and gyroscope,
 *           applies necessary conversions, and stores the values in global variables.
 * Returns:  none
 */
void mpu6050_read_data(void) {
    uint8_t buffer[6];

    // Read accelerometer values and convert to g
    twi_readfrom_mem_into(MPU6050_ADDRESS, ACCEL_XOUT_H, buffer, 6);
    accel_values[0] = ((float)((buffer[0] << 8) | buffer[1])) / 4096 - 0.07;
    accel_values[1] = ((float)((buffer[2] << 8) | buffer[3])) / 4096 + 0.02;
    accel_values[2] = ((float)((buffer[4] << 8) | buffer[5])) / 4096 + 0.07;

    // Read gyroscope values and convert to degrees per second (°/s)
    twi_readfrom_mem_into(MPU6050_ADDRESS, GYRO_XOUT_H, buffer, 6);
    gyro_values[0] = ((float)((buffer[0] << 8) | buffer[1])) / 65.5;
    gyro_values[1] = ((float)((buffer[2] << 8) | buffer[3])) / 65.5;
    gyro_values[2] = ((float)((buffer[4] << 8) | buffer[5])) / 65.5;
}


/*
 * Function: mpu6050_calibrate()
 * Purpose:  Calibrates the gyroscope by taking multiple readings and averaging them.
 * Returns:  none
 */
void mpu6050_calibrate(void) {
    gyro_x_cal = 0;
    gyro_y_cal = 0;
    gyro_z_cal = 0;

    for (int i = 0; i < calibrate_time; i++) {
        mpu6050_read_data();
        gyro_x_cal += gyro_values[0];
        gyro_y_cal += gyro_values[1];
        gyro_z_cal += gyro_values[2];
    }
    gyro_x_cal /= calibrate_time;
    gyro_y_cal /= calibrate_time;
    gyro_z_cal /= calibrate_time;
}


/*
 * Function: calculate_angles()
 * Purpose:  Calculates pitch and roll angles using data from the gyroscope and accelerometer,
 *           then combines them using a complementary filter.
 * Returns:  none
 */
float calculate_angles(void) {
    // Apply gyroscope calibration offsets
    gyro_values[0] -= gyro_x_cal;
    gyro_values[1] -= gyro_y_cal;
    gyro_values[2] -= gyro_z_cal;

    // Calculate pitch and roll angles from gyroscope
    angle_pitch += gyro_values[0] * 0.004;
    angle_roll += gyro_values[1] * 0.004;

    // Calculate pitch and roll angles from accelerometer
    float acc_total_vector = sqrt((accel_values[0] * accel_values[0]) + 
                                  (accel_values[1] * accel_values[1]) + 
                                  (accel_values[2] * accel_values[2]));

    if (acc_total_vector > 0.0001) { // Prevent division by zero
        float angle_pitch_acc = atan2(accel_values[1], accel_values[2]) * (180 / M_PI);
        float angle_roll_acc = atan2(accel_values[0], accel_values[2]) * (180 / M_PI);

        // Complementary filter
        angle_pitch = angle_pitch * 0.96 + angle_pitch_acc * 0.04;
        angle_roll = angle_roll * 0.96 + angle_roll_acc * 0.04;

    }
    return angle_roll;
}

