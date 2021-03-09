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

typedef void (*HAL_InterruptHandler)(void);

static HAL_InterruptHandler timerCallback;

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

int hal_configure1msInterrupt(void (*interruptHandler)(void))
{
    timerCallback = interruptHandler;
    
    initTimer1mSec(timerHandler);
    
    return 0;
}


static HAL_InterruptHandler interruptPinCallback;

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

int hal_configureInterruptPin(void (*interruptHandler)(void))
{
    interruptPinCallback = interruptHandler;
    
    initPorts(interruptPinHandler);
    
    return 0;
}

int hal_i2cRead(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
    return i2cRead(slaveAddr, reg, data, length);
}

int hal_i2cWrite(uint8_t slaveAddr, uint8_t reg, uint8_t *data, uint16_t length)
{
    return i2cWrite(slaveAddr, reg, data, length);
}

