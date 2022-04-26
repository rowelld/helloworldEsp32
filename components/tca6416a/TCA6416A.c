#include "TCA6416A.h"
#include "TCA6416A_i2c.h"
#include "esp_err.h"

esp_err_t TCA6416AInitDesc(i2c_dev_t *dev, i2c_port_t port, uint8_t addr, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    dev->port = port;
    dev->addr = addr;
    dev->cfg.sda_io_num = sda_gpio;
    dev->cfg.scl_io_num = scl_gpio;
    dev->cfg.master.clk_speed = 400000;

    return i2c_dev_create_mutex(dev);
}

esp_err_t TCA6416AFreeDesc(i2c_dev_t *dev)
{
    return i2c_dev_delete_mutex(dev);
}

void TCA6416AInitDefault(TCA6416ARegs *Regs)
{
    Regs->Output.all = 0x0000;
    Regs->PolarityInversion.all = 0x0000;
    Regs->Config.all = 0xFFFF;
}

unsigned char TCA6416AInitI2CReg(i2c_dev_t *dev, TCA6416ARegs *Regs)
{
    unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
    if (I2C_Write(&dev, 2, TCA6416A_CONFIG_REG0, (unsigned char *)&Regs->Config) != 0)
        return_Value = I2C_OPERATION_FAIL;

    if (I2C_Write(&dev, 2, TCA6416A_OUTPUT_REG0, (unsigned char *)&Regs->Output) != 0)
        return_Value = I2C_OPERATION_FAIL;

    if (I2C_Write(&dev, 2, TCA6416A_POLARITY_REG0, (unsigned char *)&Regs->PolarityInversion) != 0)
        return_Value = I2C_OPERATION_FAIL;

    return return_Value;
}

void TCA6416AReadInputReg(i2c_dev_t *dev, TCA6416ARegs *Regs)
{
    I2C_Read(&dev, 2, TCA6416A_INPUT_REG0, (unsigned char *)&Regs->Input);
}

void TCA6416AReadOuputReg(i2c_dev_t *dev, TCA6416ARegs *Regs)
{
    I2C_Read(&dev, 2, TCA6416A_OUTPUT_REG0, (unsigned char *)&Regs->Output);
}

void TCA6416AWriteOutputPin(i2c_dev_t *dev, TCA6416APins Pin, uint8_t state)
{
    union TCA6416A_uOutput reg_data;
    I2C_Read(&dev, 2, TCA6416A_OUTPUT_REG0, (unsigned char *)&reg_data.all);
    if (state)
    {
        reg_data.all |= 1 << Pin;
    }
    else
    {
        reg_data.all &= ~(1 << Pin);
    }
    I2C_Write(&dev, 2, TCA6416A_OUTPUT_REG0, (unsigned char *)&reg_data);
}

unsigned char TCA6416AReadReg(i2c_dev_t *dev, unsigned char regaddress)
{
    return (unsigned char)I2C_Read_Byte(&dev, regaddress);
}

void TCA6416AWriteConfig(i2c_dev_t *dev, TCA6416ARegs *Regs)
{
    I2C_Write(&dev, 2, TCA6416A_CONFIG_REG0, (unsigned char *)&Regs->Config);
}

void TCA6416AWriteOutput(i2c_dev_t *dev, TCA6416ARegs *Regs)
{
    I2C_Write(&dev, 2, TCA6416A_OUTPUT_REG0, (unsigned char *)&Regs->Output );
}

void TCA6416AWritePolarity(i2c_dev_t *dev, TCA6416ARegs *Regs)
{
    I2C_Write(&dev, 2, TCA6416A_POLARITY_REG0, (unsigned char *)&Regs->PolarityInversion);
}