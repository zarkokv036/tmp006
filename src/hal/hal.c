/**
* @file hal.c
* @brief init of platform
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h" 

int hal_init(void)
{
    initSystemClock_40MHz();
    enablePeripheralsClock();
    initI2c();
    initUartPrintf();
    
    return 0;
}

int hal_configure1msInterrupt(void (*interruptHandler)(void))
{
    initTimer1mSec(interruptHandler);
    
    return 0;
}

int hal_configureInterruptPin(void (*interruptHandler)(void))
{
    initPorts(interruptHandler);
    
    return 0;
}




