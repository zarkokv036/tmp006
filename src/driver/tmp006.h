/**
* @file tmp006.h
* @brief Platform-independent driver for TI TMP006
*
* @author Zarko Milojicic
*/

#ifndef TMP006_H
#define TMP006_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**@{ TMP006 register map */
#define TMP006_VOBJECT          0x00      
#define TMP006_TEMP_AMBIENT     0x01
#define TMP006_CONFIG           0x02
#define TMP006_MANUFACTURER_ID  0xFE
#define TMP006_DEVICE_ID        0xFF
/**@}*/

/**@{ Register masks */
#define TMP006_RST_MASK                0x8000
#define TMP006_MOD_MASK                0x7000
#define TMP006_CR_MASK                 0x0E00
#define TMP006_DRDY_EN_MASK            0x0100
#define TMP006_DRDY_RESULT_READY_MASK  0x0080

/**@}*/

#define TMP006_MANUF_ID_VALUE           0x5449
#define TMP006_DEVICE_ID_VALUE          0x0067
/**
* @brief State of ADR0 and ADR1 pins.
*/
enum TMP006_PinState
{
    TMP006_PIN_LOW,
    TMP006_PIN_HIGH,
    TMP006_PIN_SDA,
    TMP006_PIN_SCL
};

/**
* @brief Configuration of conversion rate.
*/
enum TMP006_ConversionRate
{
    TMP006_CONVERSION_RATE_4_CONV_PER_SEC   = (0 << 9),         //**< 4 conv/sec */
    TMP006_CONVERSION_RATE_2_CONV_PER_SEC   = (1 << 9),  //**< 2 conv/sec */
    TMP006_CONVERSION_RATE_1_CONV_PER_SEC   = (2 << 9),  //**< 1 conv/sec */
    TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC  = (3 << 9),  //**< 0.5 conv/sec */
    TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC = (4 << 9)   //**< 0.25 conv/sec */   
};

/**
* @brief Configuration of power mode.
*/
enum TMP006_OperationMode
{
    TMP006_POWER_DOWN = (0 << 12),
    TMP006_CONTINUOUS_CONVERSION = (7 << 12)
};

/**
* @brief Configuration of DRDY pin 
* If this mode is enabled pin is set when result of conversion is ready.
*/
enum TMP006_DRDY_pinMode
{
    TMP006_DRDY_PIN_OFF = (0 << 8) ,
    TMP006_DRDY_PIN_ON  = (1 << 8)
};

/**
* @brief TMP006 device structure
*/
typedef struct TMP006_Device
{
    int (*i2cRead)(uint8_t addr,
                   uint8_t reg,
                   uint8_t *data,
                   uint16_t length); //**< Pointer to low level I2C read function */
    int (*i2cWrite)(uint8_t addr,
                    uint8_t reg, 
                    uint8_t *data, 
                    uint16_t length);
    
    uint8_t  i2cAddress; /**< I2C address depended on ADR0 and ADR1 pin */
} TMP006_Device;

/**
* @brief Initialize TMP006 device structure 
* 
* @param dev Pointer to TMP006 device structure
* @param A0state State of pin A0
* @param A1state State of pin A1
* @return 0 on success

* @returns -EINVAL if either the read or the write callback is NULL
* @returns error code on failure
* 
* @note Before you use this function you need to initialize i2cRead() and i2cWrite()
* functions from TMP006_Device structure
*/
int tmp006_init(TMP006_Device *dev,enum TMP006_PinState A0state, enum TMP006_PinState A1state);

/**
* @brief Read data from TMP006 register.
* 
* @param dev Pointer to the TMP006 device structure
* @param reg Adress of register from which you want to read.
* @param data Pointer to a 2 bytes where data will be stored.
* 
* @returns 0 on success or an error code
*/
int tmp006_read(TMP006_Device *dev, uint8_t reg, uint16_t *data);

/**
* @brief Write data to TMP006 register
*
* @param dev Pointer to the TMP006 device structure
* @param reg Adress of register to which you want to write.
* @param data Pointer to a 2 bytes which you want to send.
* 
* @returns 0 on success or an error code
*/
int tmp006_write(TMP006_Device *dev, uint8_t reg, uint16_t *data);

/**
* @brief Configure the conversion rate of the TMP006.
*
* After performing configuration result can be taken.
*
* @param dev Pointer to the TMP006 device structure
* @param rate Rate of conversion.
*
* @returns 0 on success or an error code
*/
int tmp006_configConvRate(TMP006_Device *dev, enum TMP006_ConversionRate rate);

/**
* @brief Configure DRDY pin 
*
* If this mode is enabled DRDY pin is set when result of conversion is ready.
* If disabled you check if result is ready by checking bit in config register.
*
* @param dev Pointer to the TMP006 device structure
* @param drdyPin Mode of DRDY pin.
*
* @returns 0 on success or an error code
*/
int tmp006_drdyPinConfig(TMP006_Device *dev, enum TMP006_DRDY_pinMode drdyPin);

/**
* @brief Reset the TMP006.
* 
* After reset all bits in configuration register are set to their default value.
* Default values means sensor is in normal operation mode and performs one conversion
* of temperature per second, DRDY pin is disabled.
*
* @param dev Pointer to the TMP006 device structure
* 
* @returns 0 on success or an error code
*/
int tmp006_resetDevice(TMP006_Device *dev);

/**
* @brief Read Ambient temperature.
*
* @param[in] dev Pointer to the TMP006 device structure.
* @param[out] temperature Pointer to a variable where temperature will be stored.
*
* @returns 0 on success or an error code
*
* @note If you multiply output value by (1/32) you will get
* value of temperature in celsius degree. Range[+150 C , -50 C]
* 
*/
int tmp006_readTemp(TMP006_Device *dev, int16_t *temperature);

/**
* @brief Read Voltage from sensor.
* 
* Voltage is needed for calculation of the object temperature. 
*
* @param[int] dev Pointer to the TMP006 device structure
* @param[out] data Pointer to variable where voltage will be stored.
*
* @returns 0 on success or an error code
* 
* @note If you multiply output value stored in data by 5.12 you will get
* value of voltage in mV. Range[+5.12mV , -5.12mV]
*/
int tmp006_readVoltage(TMP006_Device *dev, int16_t *voltage);

/**
* @brief Set the TMP006 operation mode.
* 
* Initiating power-down has an immediate effect; it aborts the current conversion 
* and puts the device into a low-power shutdown mode.
*
* @param dev Pointer to the TMP006 device structure
* @param mode Operation mode of the TMP006.
* 
* @returns 0 on success or an error code
*/
int tmp006_operationMode(TMP006_Device *dev, enum TMP006_OperationMode mode);

/**
* @brief Check if result is ready 
*
* @param dev Pointer to the TMP006 device structure
* @param isReady pointer to bool that says is result ready or not.
*
* @returns 0 on success or an error code
*/
int tmp006_isResultReady(TMP006_Device *dev, bool *isReady);

#ifdef __cplusplus
}
#endif

#endif //TMP006_H
