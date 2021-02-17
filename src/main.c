/**
* @file main.c
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"
#include "tmp006.h"


//static uint8_t pinState = 0;
uint8_t flagReady = 0;
int16_t testTmp006Read = 0, testTmp006init = 0;
uint16_t readManufID = 0, readConfig = 0;
int16_t temprature = 0;
float tempInC = 0;

void togglePortF(void);
void resultReady(void);

int main(void)
{
    
    
    TMP006_Device senzor = {
        .i2cRead = i2cRead,
        .i2cWrite = i2cWrite   
    };

    testTmp006init = tmp006_init(&senzor, TMP006_PIN_LOW, TMP006_PIN_LOW);
    
    initSystemClock_40MHz();
    enablePeripheralsClock();
    initPorts(resultReady);
    initI2c(senzor.i2cAddress);
    initTimer1sec(togglePortF);
     //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    //tmp006_resetDevice(&senzor);
    

    testTmp006Read = tmp006_read(&senzor, TMP006_MANUFACTURER_ID, &readManufID); //PRVO NEK TI DOBRO OCITA VREDNOST ID-A
    testTmp006Read = tmp006_resetDevice(&senzor);
    testTmp006Read = tmp006_read(&senzor, TMP006_CONFIG, &readConfig); // 0x7400
    
    testTmp006Read = tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
    testTmp006Read = tmp006_read(&senzor, TMP006_CONFIG, &readConfig); // 0x7200
    
    testTmp006Read = tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    testTmp006Read = tmp006_read(&senzor, TMP006_CONFIG, &readConfig); // 0x7300
    while(1)
    {
        
        if (flagReady)
        {
            tmp006_readTemp(&senzor, &temprature);
            tempInC = ((float)temprature * 0.03125);
            flagReady = 0;
        }
        
        if (tempInC > 20)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); //LED ON
        }
        else 
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); //LED OFF
        }
        
    }

    return 0;
}

void resultReady(void)
{
    GPIOIntClear(GPIO_PORTA_BASE, GPIO_INT_PIN_2);
    flagReady = 1;
}
//function is called every sec
void togglePortF(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

//    int32_t pinState;
//    pinState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
//    pinState = (pinState ^ GPIO_PIN_1) & 0xFF;
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, pinState);
    
    
}
