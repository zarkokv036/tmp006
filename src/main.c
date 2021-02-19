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
            UARTprintf("test fail \n");  \
        }                                \
        else                             \
        {                                \
            UARTprintf("test pass \n");  \
        }                                \
                                         \
                                         \
    } while (0)
    
    
#define TMP006_REG_TEST(mask,checkValue)                         \
    do                                                           \
    {                                                            \
       uint16_t valueOfReg = (readReg(&senzor, TMP006_CONFIG));  \
       valueOfReg &= mask;                                       \
       TMP006_TEST(valueOfReg != checkValue);                    \
    }while(0)                                                    \

    
//static uint8_t pinState = 0;
uint8_t flagReady = 0, flagUartSend = 0;
int16_t testTmp006Read = 0, testTmp006init = 0;
//uint16_t readManufID = 0, readConfig = 0;
int16_t temprature = 0;
float tempInC = 0;
uint8_t uartReceived[4];

void togglePortF(void);
void resultReady(void);
void uartHandler(void);
uint16_t readReg(TMP006_Device *dev, uint8_t reg);

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
    //initUart0(uartHandler);
    initUartPrintf();
     //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    //tmp006_resetDevice(&senzor);
    uint16_t regValue;
    
    UARTprintf("~~~TEST~~~ \n");
    regValue = readReg(&senzor, TMP006_MANUFACTURER_ID);
    TMP006_TEST(regValue != TMP006_MANUF_ID_VALUE);
    
    for(uint32_t i = TMP006_CONVERSION_RATE_4_CONV_PER_SEC; i<= TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC; i++)
    {
        switch(i)
        {
            case TMP006_CONVERSION_RATE_4_CONV_PER_SEC:
            {
                tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
                TMP006_REG_TEST (TMP006_CR_MASK, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
//                regValue = readReg(&senzor, TMP006_CONFIG);
//                regValue &= TMP006_CR_MASK;
//                TMP006_TEST(regValue != TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
                break;
            }
            case TMP006_CONVERSION_RATE_2_CONV_PER_SEC:
            {
                tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
                TMP006_REG_TEST (TMP006_CR_MASK, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
                break;
            }
            case TMP006_CONVERSION_RATE_1_CONV_PER_SEC:
            {
                tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
                TMP006_REG_TEST (TMP006_CR_MASK, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
                break;
            }
            case TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC:
            {
                tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
                TMP006_REG_TEST (TMP006_CR_MASK, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
                break;
            }
            case TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC:
            {
                tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
                TMP006_REG_TEST (TMP006_CR_MASK, TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
                break;
            }
        }            
    }
    
    
//    testTmp006Read = tmp006_read(&senzor, TMP006_MANUFACTURER_ID, &readManufID); //PRVO NEK TI DOBRO OCITA VREDNOST ID-A
//    testTmp006Read = tmp006_resetDevice(&senzor);
//    testTmp006Read = tmp006_read(&senzor, TMP006_CONFIG, &readConfig); // 0x7400
//    
//    testTmp006Read = tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
//    testTmp006Read = tmp006_read(&senzor, TMP006_CONFIG, &readConfig); // 0x7200
//    
//    testTmp006Read = tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
//    testTmp006Read = tmp006_read(&senzor, TMP006_CONFIG, &readConfig); // 0x7300
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
//        if(flagUartSend)
//        {
//            flagUartSend = 0;
//            uint8_t s = 6;
//            //UARTCharPut(UART0_BASE, s);//samo proba
//            UARTprintf("PROBA ");
//        }
        
    }

    return 0;
}

uint16_t readReg(TMP006_Device *dev, uint8_t reg)
{
    uint16_t Value, status;
    status = tmp006_read(dev, reg, &Value);
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

void uartHandler(void)
{
    uint32_t intStatus;
    
    intStatus = UARTIntStatus(UART0_BASE, true);
    
    UARTIntClear(UART0_BASE, intStatus); //clear the asserted interrupts
    uint16_t i = 0; 
    //rx interrupt happens when there is 4 byte in rx fifo    
    while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
    {
         uartReceived[i] = UARTCharGetNonBlocking(UART0_BASE);
         i++;
    }
}