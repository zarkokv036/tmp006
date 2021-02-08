/**
* @file tmp006.h
* @brief Platform-independent driver for TI TMP006
*
* @author Zarko Milojicic
*/

#include "tmp006.h"

#include <errno.h>
#include <stddef.h>


/**
 * Helper macro for error handling
 *
 * @note Guaranteed to only perform one evaluation of `status`
 */
#define TMP006_FAIL_UNLESS_OK(status) \
    do                                  \
    {                                   \
        int temp = (status);            \
        if (0 != temp)                  \
        {                               \
            return temp;                \
        }                               \
    } while (0)
/*
* Helper macro for parametar checking
*/    
#define TMP006_CHECK_PARAM(expr) \
    do                           \
    {                            \
        if (!(expr))             \
        {                        \
            return -EINVAL;      \
        }                        \
    } while (0)
    
/**
* @brief TMP006 7-bit I2C address 
* 
* The i2c address of TMP006 depends on the state of ADR0 and ADR1 pin.
* See the documentation of TMP006 for details.
*
* This is just base address for accessing of sensor registers.
*/
#define TMP006_I2C_BASE_ADDRESS 0x40
    
/**
* @brief Set the i2c address depending on pin state
* 
* @param A0state state of pin A0
* @param A1state state of pin A1
* @returns Final address of i2c or TMP006_A0A1_ADDR_ERROR if inputs not valid.
*/
static int setI2cAddress(uint8_t *address ,enum TMP006_PinState A0state, enum TMP006_PinState A1state)
{
    if (A0state == TMP006_PIN_LOW)
    {
        if (A1state == TMP006_PIN_LOW)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 0;
        }
        else if (A1state == TMP006_PIN_HIGH)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 1;
        }
        else if (A1state == TMP006_PIN_SDA)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 2;
        }
        else if (A1state == TMP006_PIN_SCL)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 3;
        }
        else
        {
            return -EINVAL;
        }
        
    }
    else if (A0state == TMP006_PIN_HIGH)
    {
        if (A1state == TMP006_PIN_LOW)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 4;
        }
        else if (A1state == TMP006_PIN_HIGH)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 5;
        }
        else if (A1state == TMP006_PIN_SDA)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 6;
        }
        else if (A1state == TMP006_PIN_SCL)
        {
            *address = TMP006_I2C_BASE_ADDRESS | 7;
        }
        else
        {
            return -EINVAL;
        }
    }
    else
    {
        return -EINVAL;
    }
    
    return 0;
}


int tmp006_init(TMP006_Device *dev,enum TMP006_PinState A0State, enum TMP006_PinState A1State)
{
    TMP006_CHECK_PARAM((dev == NULL) || (dev->i2cRead == NULL) || (dev->i2cWrite == NULL));

    
    int status = setI2cAddress(&dev->i2cAddress, A0State, A1State);
    TMP006_FAIL_UNLESS_OK(status);
    
    return 0;
}

int tmp006_read(TMP006_Device *dev, uint8_t reg, uint16_t *data)
{
    uint8_t value[2];
    TMP006_CHECK_PARAM((dev == NULL) || (data == NULL));
 
    int status = dev->i2cRead(dev->i2cAddress, reg, value, 2);
    TMP006_FAIL_UNLESS_OK(status);
    
    *data = (uint16_t)(((uint16_t)value[0] << 8) | value[1]); //MSB is received first
    return 0;
}

int tmp006_write(TMP006_Device *dev, uint8_t reg, uint16_t *data)
{
    uint8_t value[2];
    TMP006_CHECK_PARAM((dev == NULL) || (data == NULL));

    
    value[0] = (uint8_t)(*data >> 8);
    value[1] = (uint8_t)(*data & 0x00FF);
    
    int status = dev->i2cWrite(dev->i2cAddress, reg, value, 2);
    TMP006_FAIL_UNLESS_OK(status);
    
    return 0;
}

int tmp006_configConvRate(TMP006_Device *dev, enum TMP006_ConversionRate rate)
{
    TMP006_CHECK_PARAM((dev == NULL) || (rate > TMP006_CONVERSION_RATE_025));
    
    uint16_t currentValue;
    int status = tmp006_read(dev, TMP006_CONFIG, &currentValue);
    TMP006_FAIL_UNLESS_OK(status);
    
    currentValue &= (~TMP006_CR_MASK);
    currentValue |= rate;
    
    status = tmp006_write(dev, TMP006_CONFIG, &currentValue);
    TMP006_FAIL_UNLESS_OK(status);
    
    return 0;
}

int tmp006_drdyPinConfig(TMP006_Device *dev, enum TMP006_DRDY_pinMode drdyPin)
{
    TMP006_CHECK_PARAM((dev == NULL) || (drdyPin > TMP006_DRDY_PIN_ON));
    
    uint16_t currentValue;
    int status = tmp006_read(dev, TMP006_CONFIG, &currentValue);
    TMP006_FAIL_UNLESS_OK(status);
    
    currentValue &= (~TMP006_DRDY_EN_MASK);
    currentValue |= drdyPin;
    
    status = tmp006_write(dev, TMP006_CONFIG, &currentValue);
    TMP006_FAIL_UNLESS_OK(status);
    
    return 0;  
}

int tmp006_resetDevice(TMP006_Device *dev)
{
    TMP006_CHECK_PARAM(dev == NULL);
    
    uint16_t val = TMP006_RST_MASK;
    int status = tmp006_write(dev, TMP006_CONFIG, &val);
    TMP006_FAIL_UNLESS_OK(status);
    
    return 0;
}

int tmp006_operationMode(TMP006_Device *dev, enum TMP006_OperationMode mode)
{
    TMP006_CHECK_PARAM((dev == NULL) || (mode > TMP006_CONTINUOUS_CONVERSION));
    
    uint16_t currentValue;
    int status = tmp006_read(dev, TMP006_CONFIG, &currentValue);
    TMP006_FAIL_UNLESS_OK(status);
    
    currentValue &= (~TMP006_MOD_MASK);
    currentValue |= mode;
    
    status = tmp006_write(dev, TMP006_CONFIG, &currentValue);
    TMP006_FAIL_UNLESS_OK(status);
    
    return 0;
}

int tmp006_isResultReady(TMP006_Device *dev, bool *isReady)
{
    TMP006_CHECK_PARAM(dev == NULL);

    uint16_t currentValue;
    int status = tmp006_read(dev, TMP006_CONFIG, &currentValue);
    TMP006_FAIL_UNLESS_OK(status);
    
    *isReady = currentValue & TMP006_DRDY_RESULT_READY_MASK;
    
    return 0;
}

int tmp006_readTemp(TMP006_Device *dev, int16_t *temperature)
{
    TMP006_CHECK_PARAM((dev == NULL) || (temperature == NULL));
    
    int status = tmp006_read(dev, TMP006_TEMP_AMBIENT, (uint16_t *)temperature);
    TMP006_FAIL_UNLESS_OK(status);

    return 0;
}

int tmp006_readVoltage(TMP006_Device *dev, int16_t *voltage)
{
    TMP006_CHECK_PARAM((dev == NULL) || (voltage == NULL));

    int status = tmp006_read(dev, TMP006_VOBJECT, (uint16_t *)voltage);
    TMP006_FAIL_UNLESS_OK(status);
    
    return 0;
}







