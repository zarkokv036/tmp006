/**
* @file hal.h
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/



#ifndef HAL_H
#define  HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief initialization of tm4c123 
*/
int hal_init(void);

int hal_configure1msInterrupt(void (*interruptHandler)(void));

int hal_configureInterruptPin(void (*interruptHandler)(void));


#ifdef __cplusplus
}
#endif

#endif //HAL_H