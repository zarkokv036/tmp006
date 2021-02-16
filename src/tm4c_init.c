/**
* @file tm4c_init.c
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"


void initSystemClock_40MHz(void)
{
    SysCtlClockSet(SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN );
}

void enablePeripheralsClock(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
    {
    }
}

void initPorts(void)
{
    GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3));
   
    
//    GPIODirModeSet(GPIO_PORTF_BASE, (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_OUT);
//    GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
//    GPIOPadConfigSet(GPIO_PORTF_BASE,(GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3), GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
}

void initTimer(void (*pfnHandler)(void))
{
    // Enable the Timer0 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    //
    // Wait for the Timer0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }
    //
    // Configure Timer0 as a full-width periodic timer
    //
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    
    
    //
    // Set the count time for the the periodic timer (TimerA).
    //1 sec
    TimerLoadSet(TIMER0_BASE, TIMER_A, 40000000);
    
    TimerIntRegister(TIMER0_BASE, TIMER_A, pfnHandler);
    
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);///nisam siguran
    
    
    TimerEnable(TIMER0_BASE, TIMER_A);
    
}

