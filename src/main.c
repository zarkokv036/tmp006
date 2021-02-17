/**
* @file main.c
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"
#include "tmp006.h"


//static uint8_t pinState = 0;
static uint8_t flagRead = 0;
int16_t testTmp006Read = 0, testTmp006init = 0;
uint16_t testVrednost = 0;
int16_t temprature = 0;
float tempInC;

void togglePortF(void);

int main(void)
{
    
    
    TMP006_Device senzor = {
        .i2cRead = i2cRead,
        .i2cWrite = i2cWrite   
    };

    testTmp006init = tmp006_init(&senzor, TMP006_PIN_LOW, TMP006_PIN_LOW);
    
    initSystemClock_40MHz();
    enablePeripheralsClock();
    initPorts();
    initI2c(senzor.i2cAddress);

    tmp006_resetDevice(&senzor);
    initTimer(togglePortF);
     //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    //tmp006_resetDevice(&senzor);
    

    testTmp006Read = tmp006_read(&senzor, TMP006_DEVICE_ID, &testVrednost); //PRVO NEK TI DOBRO OCITA VREDNOST ID-A
    while(1)
    {
        
        if (flagRead)
        {
            tmp006_readTemp(&senzor, &temprature);
//            tempInC = temprature * (1/32);
            flagRead = 0;
        }
        
        if (temprature > 1)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
        }
        else 
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        }
        
    }

    return 0;
}

void togglePortF(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    flagRead = 1;
    
//    int32_t pinState;
//    pinState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
//    pinState = (pinState ^ GPIO_PIN_1) & 0xFF;
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, pinState);
    
    
}
