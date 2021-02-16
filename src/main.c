/**
* @file main.c
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"
#include "tm4c_i2c.h"
#include "tmp006.h"


//static uint8_t pinState = 0;
static unsigned long Led;
void togglePortF(void);

int main(void)
{
    TMP006_Device senzor;
    senzor.i2cRead = i2cRead;
    senzor.i2cWrite = i2cWrite;
    
    initSystemClock_40MHz();
    enablePeripheralsClock();
    initPorts();
    i2c_Initialize();
    
//    tmp006_init(&senzor, TMP006_PIN_LOW, TMP006_PIN_LOW);
//    tmp006_resetDevice(&senzor);
    initTimer(togglePortF);
     //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    
    while(1)
    {
        
        
    }

    return 0;
}

void togglePortF(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    int32_t pinState;
    pinState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
    pinState = (pinState ^ GPIO_PIN_1) & 0xFF;
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, pinState);
}
