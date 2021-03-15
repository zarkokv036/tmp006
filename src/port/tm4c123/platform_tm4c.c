/**
* @file platform_tm4c.c
* @brief init of platform
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"
#include "platform.h"

int platform_init(void)
{
    initSystemClock_40MHz();
    enablePeripheralsClock();
    initI2c();
    initUartPrintf();
    
    return 0;
}

typedef void (*platform_InterruptHandler)(void);

static platform_InterruptHandler timerCallback;

/**
* @brief Timer interrupt handler
* @note TimerIntClear() is optionl and it depends on used platform.
*/
static void timerHandler(void)
{
    //clear interrupt 
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    
    timerCallback();    
}

int platform_configure1msInterrupt(void (*interruptHandler)(void))
{
    timerCallback = interruptHandler;
    
    initTimer1mSec(timerHandler);
    
    return 0;
}

static platform_InterruptHandler interruptPinCallback;

/**
* @brief Falling edge pin interrupt handler
* @note GPIOIntClear() is optionl and it depends on used platform.
*/
static void interruptPinHandler (void)
{
    //clear interrupt 
    GPIOIntClear(GPIO_PORTA_BASE, GPIO_INT_PIN_2);
    
    interruptPinCallback();
}

int platform_configureInterruptPin(void (*interruptHandler)(void))
{
    interruptPinCallback = interruptHandler;
    
    initPorts(interruptPinHandler);
    
    return 0;
}

int platform_i2cRead(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
    return i2cRead(slaveAddr, reg, data, length);
}

int platform_i2cWrite(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
    return i2cWrite(slaveAddr, reg, data, length);
}

