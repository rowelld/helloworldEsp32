#ifndef COMPONENTS_TCA6416A_TCA6416A_I2C
#define COMPONENTS_TCA6416A_TCA6416A_I2C

#include <stdint.h>
#include "i2cdev.h"

#define I2C_OPERATION_FAIL -1
#define I2C_OPERATION_SUCCESSFUL 0

esp_err_t I2C_Write(i2c_dev_t *dev, uint8_t byte_count, uint8_t reg_addr, uint8_t *reg_data);
esp_err_t I2C_Read(i2c_dev_t *dev, uint8_t byte_count, uint8_t reg_addr, uint8_t *reg_data);
esp_err_t I2C_Read_Byte(i2c_dev_t *dev, uint8_t reg_addr);

#endif /* COMPONENTS_TCA6416A_TCA6416A_I2C */
