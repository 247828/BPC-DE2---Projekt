#ifndef MPU6050_H
#define MPU6050_H


/**
 * @name MPU6050 I2C Address
 * @{
 */
#define MPU6050_ADDRESS 0x68 /**< @brief I2C address for MPU6050 */
/** @} */

/**
 * @name Register Addresses
 * @brief Register addresses for accessing MPU6050 data.
 * @{
 */
#define ACCEL_XOUT_H 0x3B    /**< @brief Register address for X-axis accelerometer data */
#define GYRO_XOUT_H 0x43     /**< @brief Register address for X-axis gyroscope data */
/** @} */

/**
 * @name Function Prototypes
 * @brief Function declarations for initializing and reading MPU6050.
 * @{
 */

/**
 * @brief Initialize MPU6050 and set initial configurations.
 * @return void
 */
void mpu6050_init(void);

/**
 * @brief Read accelerometer and gyroscope data from MPU6050.
 * @return void
 */
void mpu6050_read_data(void);

/**
 * @brief Calibrate the gyroscope by computing offsets.
 * @return void
 */
void mpu6050_calibrate(void);

/**
 * @brief Calculate pitch and roll angles based on accelerometer and gyroscope data.
 * @return void
 */
float calculate_angles(void);
/** @} */

/**
 * @name Global Variables
 * @brief Global variables used for storing sensor data.
 * @{
 */
extern volatile float accel_values[3];    /**< @brief Accelerometer values (X, Y, Z) */
extern volatile float gyro_values[3];     /**< @brief Gyroscope values (X, Y, Z) */
extern volatile float angle_pitch;        /**< @brief Calculated pitch angle */
extern volatile float angle_roll;         /**< @brief Calculated roll angle */
/** @} */

/**
 * @brief Read new data from MPU6050
 * @return void
 */
void read_mpu6050(void);

#endif  // MPU6050_H

