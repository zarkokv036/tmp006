/**
* @file hal.h
* @brief init of platform
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h" 

#ifndef HAL_H
#define  HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Wraper for printf function
* @note Change the UARTprintf with appropriate name of your printf function
*/
#define PRINTF(...)   UARTprintf(__VA_ARGS__)

/**
* @brief initialization of tm4c123
* 
* This function needs to have:
* - init of system clock
* - init of I2C - max speed 400kbs
* - init of UART and others so printf function can be used
*
* @return 0 on sucess, 
*/
int hal_init(void);

/**
* @brief init of 1 ms timer
* 
* @param interruptHandler pointer to timer handler function
* @return 0 on sucess, 
*/
int hal_configure1msInterrupt(void (*interruptHandler)(void));

/**
* @brief init of falling edge interrupt pin.
*
* @param interruptHandler pointer to edge interrupt handler function
* @return 0 on sucess, 
*/
int hal_configureInterruptPin(void (*interruptHandler)(void));

/**
* @brief i2c write command
* @param slaveAddr address of slave
* @param reg address of register you want to write into
* @param data pointer to a data you want to write into slave
* @param length length of data you want to send
* @return 0 on success or error code on failure.
*/
int hal_i2cRead(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length);

/**
* @brief i2c write command
*
* @param slaveAddr address of slave
* @param reg address of register you want to write into
* @param data pointer to a data you want to write into slave
* @param length length of data you want to send
* @return 0 on success or error code on failure.
*/
int hal_i2cWrite(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length);


#ifdef __cplusplus
}
#endif

#endif //HAL_H