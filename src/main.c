/**
* @file main.c
* @brief init file for tm4c123g
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"
#include "tmp006.h"

/*
* Helper macro for parametar checking
*/    
#define TMP006_TEST(expr)                \
    do                                   \
    {                                    \
        if (expr)                        \
        {                                \
            UARTprintf("test failed \n");  \
        }                                \
        else                             \
        {                                \
            UARTprintf("test passed \n");  \
        }                                \
                                         \
                                         \
    } while (0)
    
    
#define TMP006_TEST_ASSERT(mask,checkValue)                      \
    do                                                           \
    {                                                            \
       uint16_t valueOfReg = (readReg(&senzor, TMP006_CONFIG));  \
       valueOfReg &= mask;                                       \
       TMP006_TEST(valueOfReg != checkValue);                    \
    }while(0)                                                    \

   

    
uint8_t flagReady = 0, flagUartSend = 0;
int16_t testTmp006init = 0;
int16_t temprature = 0;
float tempInC = 0;

/**
* @brief toggling LED every second with timer
*/
void togglePortF(void);

/**
* @brief handler for edge interrupt on PORTA2
*/
void resultReady(void);

/**
* @brief Read value of specified register
* @param dev Pointer to the TMP006 device structure
* @param addr address of register you want to read
*/    
uint16_t readReg(TMP006_Device *dev, uint8_t addr);

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
    initUartPrintf();
    uint16_t regValue;

    UARTprintf("~~~TEST~~~ \n");
    
    //Reading manufacture ID test
    UARTprintf("reading id test\n");
    regValue = readReg(&senzor, TMP006_MANUFACTURER_ID);
    TMP006_TEST(regValue != TMP006_MANUF_ID_VALUE);
    
    //TEST OF CONVERSION RATE CONFIGURATION
    UARTprintf("conversion rate configuration test \n");
    for(uint16_t i = 0; i <= (TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC >> 9) ; i++)
    {
        tmp006_configConvRate(&senzor, (i << 9));
        TMP006_TEST_ASSERT(TMP006_CR_MASK, (i << 9));
    }
     
    //Test of DRDY pin mode
    UARTprintf("result ready pin(DRDY) configuration test \n");
    
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    TMP006_TEST_ASSERT(TMP006_DRDY_EN_MASK, TMP006_DRDY_PIN_OFF);
    
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    TMP006_TEST_ASSERT(TMP006_DRDY_EN_MASK, TMP006_DRDY_PIN_ON);
    
    //Test of operation mode
    UARTprintf("Power mode operation test \n");
    
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    TMP006_TEST_ASSERT(TMP006_MOD_MASK, TMP006_POWER_DOWN);
    
    tmp006_operationMode(&senzor, TMP006_CONTINUOUS_CONVERSION);
    TMP006_TEST_ASSERT(TMP006_MOD_MASK, TMP006_CONTINUOUS_CONVERSION);
    
    //Reset device test
    UARTprintf("Reset operation test \n");
    tmp006_resetDevice(&senzor);
    TMP006_TEST_ASSERT(0xFFFF, 0x7400); //default value of config reg after reset is 0x7400 
    
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
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

uint16_t readReg(TMP006_Device *dev, uint8_t addr)
{
    uint16_t Value, status;
    status = tmp006_read(dev, addr, &Value);
    if(status != 0)
    {
        return status;
    }
    return Value;
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
    flagUartSend = 1;
//    int32_t pinState;
//    pinState = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
//    pinState = (pinState ^ GPIO_PIN_1) & 0xFF;
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, pinState);
}
