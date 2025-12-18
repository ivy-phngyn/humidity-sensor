#ifndef SI7021_H
#define SI7021_H

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

#define SI7021_SENSOR_ADDR          0x40     /*!< Address for SI7021 sensor */
#define I2C_MASTER_TIMEOUT_MS       1000     /*!< I2C Master Timeout */
#define MEASURE_RH_NHMM             0xF5     /*!< Command to measure relative humidity - no clock stretching */
#define I2C_MASTER_SCL_IO           22       /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_FREQ_HZ          100000 /*!< I2C master clock frequency */

/**
* Initializes and configures ESP32 to be master & SI7021 sensor to be slave on I2C bus
* 
* @return ESP error code
*/
esp_err_t si7021_init(void);

/**
* Takes relative humidity measurement from SI7021
* Converts float measurement to string for logging purposes
* 
* @param[in,out ] humidity char pointer to hold humidity measurement
* @return ESP error code
*/
esp_err_t si7021_read_humidity(float *humidity);

/**
* Deletes I2C bus
* 
* @return ESP error code
*/
esp_err_t si7021_deinit(void);

#endif // SI7021_H
