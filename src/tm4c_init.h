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

#include "tm4c123gh6pm.h"


/**
* @brief set the system clock to 40MHz
*/
void initSystemClock_40MHz(void);

/**
* @brief enable clock on peripherals gpioF and uart0
*/
void enablePeripheralsClock(void);

/**
* @brief portf3,2,1 as output, pa2 input--with interupt for it
*/
void initPorts(void (*portA2IntHandler)(void));
/**
* @brief init of timer, generate interrupt every 1 sec
*/
void initTimer1sec(void (*pfnHandler)(void));


/**
* @brief init of i2c1
*/
void initI2c(uint8_t slaveAddr);

/**
* @brief i2c write command
*/
int i2cWrite(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length);

/**
* @brief i2c read command
*/
int i2cRead(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif //TM4C123_INIT_H
