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
#include "../inc/hw_memmap.h"

#include "tm4c123gh6pm.h"


/**
* @brief set the system clock to 40MHz
*/
void initSystemClock_40MHz(void);

/**
* @brief enable clock on peripherals
*/
void enablePeripheralsClock(void);

/**
* @brief portf3,2,1 as output, pa2 input--without interupt for it
*/
void initPorts(void);
/**
* @brief init of systick
*/
void initTimer(void (*pfnHandler)(void));

#ifdef __cplusplus
}
#endif

#endif //TM4C123_INIT_H
