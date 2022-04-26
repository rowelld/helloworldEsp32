
#include "TCA6416A_i2c.h"
#include "esp_err.h"

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

#define I2C_ACK_EN 1

esp_err_t I2C_Write(i2c_dev_t *dev, uint8_t byte_count, uint8_t reg_addr, uint8_t *reg_data)
{
    CHECK_ARG(dev);

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, reg_addr, reg_data, byte_count));
    I2C_DEV_GIVE_MUTEX(dev);
    // esp_err_t i2c_err;
    // i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, I2C_ACK_EN);
    // i2c_master_write_byte(cmd, reg_addr, I2C_ACK_EN);
    // i2c_master_write(cmd, reg_data, byte_count, I2C_ACK_EN);
    // i2c_master_stop(cmd);
    // i2c_err = i2c_master_cmd_begin(i2c_num, cmd, 100);
    // i2c_cmd_link_delete(cmd);

    return ESP_OK;
}

esp_err_t I2C_Read(i2c_dev_t *dev, uint8_t byte_count, uint8_t reg_addr, uint8_t *reg_data)
{
    CHECK_ARG(dev && reg_data);

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, reg_addr, reg_data, byte_count));
    I2C_DEV_GIVE_MUTEX(dev);
    // esp_err_t i2c_err;
    // i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE,
    // I2C_ACK_EN);
    // i2c_master_write_byte( cmd, reg_addr, I2C_ACK_EN );
    // i2c_master_start( cmd );
    // i2c_master_write_byte(cmd,
    //         ( slave_addr << 1) | I2C_MASTER_READ,
    //         I2C_ACK_EN);
    // i2c_master_read( cmd, reg_data, byte_count, I2C_MASTER_LAST_NACK );
    // i2c_master_stop( cmd );
    // i2c_err = i2c_master_cmd_begin( i2c_num, cmd, 100 );
    // i2c_cmd_link_delete( cmd );

    return ESP_OK;
}

esp_err_t I2C_Read_Byte(i2c_dev_t *dev, uint8_t reg_addr)
{
    // I2C_Read(&dev, 1, reg_addr, reg_addr)
    // i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE,
    // I2C_ACK_EN);
    // i2c_master_write_byte( cmd, reg_addr, I2C_ACK_EN );
    // i2c_master_start( cmd );
    // i2c_master_write_byte(cmd,
    //         ( slave_addr << 1) | I2C_MASTER_READ,
    //         I2C_ACK_EN);
    // i2c_master_read_byte( cmd, &reg_data,  I2C_MASTER_LAST_NACK );
    // i2c_master_stop( cmd );
    // i2c_master_cmd_begin( i2c_num, cmd, 100 );
    // i2c_cmd_link_delete( cmd );

    return ESP_OK;
}