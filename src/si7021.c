#include "si7021.h"

static i2c_master_bus_handle_t s_bus_handle = NULL;
static i2c_master_dev_handle_t s_dev_handle = NULL;

/**
* Reads a register from the SI7021 sensor
* 
* 1. send command
* ______________________________________________________________________
* | start | slave_addr + wr_bit + ack | measure cmd + ack       | stop |
* --------|---------------------------|-------------------------|------|
*
* 2. read data
* ___________________________________________________________________________________
* | start | slave_addr + rd_bit + ack | read data_len byte + ack(last nack)  | stop |
* --------|---------------------------|--------------------------------------|------|
*
* @param[in] dev_handle I2C master bus device handle
* @param[in] command command to send SI7021 sensor 
* @param[in, out] data data buffer for reading from SI7021 sensor
* @param[in] len size of read buffer
* @return ESP error code
*/
static esp_err_t si7021_register_read(i2c_master_dev_handle_t dev_handle, uint8_t command, uint8_t *data, size_t len)
{
    esp_err_t ret = i2c_master_transmit(dev_handle, &command, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK) return ret;
    
    vTaskDelay(pdMS_TO_TICKS(25)); // delay between write & read for accuracy
    
    ret = i2c_master_receive(dev_handle, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return ret;
}

/**
* Converts measurement read from SI7021 to percent relative humidity
* 
* @param[in] raw_value measurement from SI7021
* @return float representation of percent relative humidity obtained
*/
static float si7021_convert_rh(uint16_t raw_value)
{
    return ((125.0 * raw_value) / 65536.0) - 6.0;
}

esp_err_t si7021_init(void)
{
    if (s_bus_handle && s_dev_handle) return ESP_OK;
    
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&bus_config, &s_bus_handle);
    if (err != ESP_OK) return err;
    
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SI7021_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    err = i2c_master_bus_add_device(s_bus_handle, &dev_config, &s_dev_handle);
    return err;
}

esp_err_t si7021_read_humidity(float *humidity)
{
    if (!s_dev_handle) return ESP_ERR_INVALID_STATE;
    
    // read the data from SI7021 via I2C bus
    uint8_t data[2];
    esp_err_t ret = si7021_register_read(s_dev_handle, MEASURE_RH_NHMM, data, 2);
    if (ret != ESP_OK) return ret;
    
    // if the read was successful, then convert the measurement to relative humidity
    
    uint16_t raw_rh = (data[0] << 8) | data[1];
    *humidity = si7021_convert_rh(raw_rh);
    return ESP_OK;
}

esp_err_t si7021_deinit(void)
{
    if (s_dev_handle) {
        i2c_master_bus_rm_device(s_dev_handle);
        s_dev_handle = NULL;
    }
    if (s_bus_handle) {
        i2c_del_master_bus(s_bus_handle);
        s_bus_handle = NULL;
    }
    return ESP_OK;
}
