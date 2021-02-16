/**
* @file tm4c_i2c.h
* @brief init file for i2c
*
* @author Zarko Milojicic
*/
#ifndef TM4C123_I2C_H
#define  TM4C123_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Driver_I2C.h"


/**
* @brief init of i2c
*/
int i2c_Initialize(void);

/**
* @brief i2c write command
*/
int i2cWrite(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t length);

/**
* @brief i2c read command
*/
int i2cRead(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t length);


#ifdef __cplusplus
}
#endif

#endif //TM4C123_I2C_H





