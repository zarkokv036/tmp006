/**
* @file tm4c_init.h
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/

#ifndef TM4C123_INIT_H
#define  TM4C123_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


#include "../driverlib/sysctl.h"
#include "../driverlib/gpio.h"
#include "../driverlib/pin_map.h"
#include "../driverlib/timer.h"
#include "../driverlib/i2c.h"
#include "../inc/hw_memmap.h"
#include "../driverlib/uart.h"
#include "../utils/uartstdio.h"

#include "tm4c123gh6pm.h"


/**
* @brief set the system clock to 40MHz
*/
void initSystemClock_40MHz(void);

/**
* @brief enable clock on peripherals GPIOA, GPIOF, and uart0
*/
void enablePeripheralsClock(void);

/**
* @brief portf3,2,1 as output, pa2 input--with falling edge interupt for it
* @param portA2IntHandler pointer to a handler function of edge interrupt
*/
void initPorts(void (*portA2IntHandler)(void));

/**
* @brief init of timer, generate interrupt every 1 sec
* @param pfnHandler pointer to a handler function timer interrupt
*/
void initTimer1mSec(void (*pfnHandler)(void));

/**
* @brief init of i2c1, speed 100kbs
* @param slaveAddr address of slave
*/
void initI2c();

/**
* @brief i2c write command
* @param slaveAddr address of slave
* @param reg address of register you want to write into
* @param data pointer to a data you want to write into slave
* @param length length of data you want to send
*/
int i2cWrite(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length);

/**
* @brief i2c read command
* @param slaveAddr address of slave
* @param reg address of register you want to read
* @param data pointer to a data you want to read from
* @param length length of data you want to receive
*/
int i2cRead(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length);

/**
* @brief init of uart0 for debugging 
*/
void initUartPrintf(void);

#ifdef __cplusplus
}
#endif

#endif //TM4C123_INIT_H
