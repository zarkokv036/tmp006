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
#include "../driverlib/sysctl.h"

/**
* @brief set the system clock to 40MHz
*/
void systemClockInit_40MHz(void);

/**
* @brief enable clock on peripherals
*/
void peripheralsClockEnable(void);




#ifdef __cplusplus
}
#endif

#endif //TM4C123_INIT_H