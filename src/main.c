/**
* @file main.c
* @brief 
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"
#include "tmp006.h"

/**
* Helper macro for parametar checking
*/    
#define TMP006_TEST(expr)                \
    do                                   \
    {                                    \
        if (expr)                        \
        {                                \
            return false;                \
        }                                \
                                         \
    } while (0)
    
/**
* Helper macro for checking of CONFIG register
*/    
#define TMP006_TEST_ASSERT(mask,checkValue)                      \
    do                                                           \
    {                                                            \
       uint16_t valueOfReg = (readReg(&senzor, TMP006_CONFIG));  \
       valueOfReg &= mask;                                       \
       TMP006_TEST(valueOfReg != checkValue);                    \
    }while(0)                                                    \

   

    
static volatile uint8_t resultReadyFlag = 0;
static int16_t testTmp006init = 0;
static int16_t temprature = 0;
static float tempInC = 0;
static volatile uint32_t msCounter = 0, resultCounter = 0;

static TMP006_Device senzor = {
        .i2cRead = i2cRead,
        .i2cWrite = i2cWrite   
    };
    
/**
* @brief check if temperature is in reasonable values
* @return true if temp is between 
*/
bool checkTemperatureValue();

/**
* @brief handler for timer, 1 ms timer
*/
void timerHandler(void);

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

/**
* @brief run tests and print if success or fail
* @param testCase Pointer to function of test case
* @param nameOfTest pointer to name of test
*/
void runTest(bool (*testCase)(void), const char *nameOfTest);

/**
* @brief test if values of commands are written into config reg
*/
bool test_writeIntoConfig(void);

/**
* @brief reading manufacturer id from device
*/
bool test_readManufId(void);

/**
* @brief test of 1 conv/sec with interrupt pin enabled
*/
bool test_readTemp1convPerSecIntOn(void);

/**
* @brief test of 2 conv/sec with interrupt pin enabled
*/
bool test_readTemp2convPerSecIntOn(void);

/**
* @brief test of 4 conv/sec with interrupt pin enabled
*/
bool test_readTemp4convPerSecIntOn(void);

/**
* @brief test of 0.5 conv/sec with interrupt pin enabled
*/
bool test_readTemp05convPerSecIntOn(void);

/**
* @brief test of 0.25 conv/sec with interrupt pin enabled
*/
bool test_readTemp025convPerSecIntOn(void);

/**
* @brief test of 1 conv/sec with interrupt pin enabled
*/
bool test_readTemp1convPerSecIntOff(void);

/**
* @brief test of 2 conv/sec with interrupt pin enabled
*/
bool test_readTemp2convPerSecIntOff(void);

/**
* @brief test of 4 conv/sec with interrupt pin enabled
*/
bool test_readTemp4convPerSecIntOff(void);

/**
* @brief test of 0.5 conv/sec with interrupt pin enabled
*/
bool test_readTemp05convPerSecIntOff(void);

/**
* @brief test of 0.25 conv/sec with interrupt pin enabled
*/
bool test_readTemp025convPerSecIntOff(void);

/**
* @brief test power down operation mode with interrupt enabled (should not give any result)
*/
bool test_powerDownModeIntOn(void);

/**
* @brief test power down operation mode with interrupt disabled (should not give any result)
*/
bool test_powerDownModeIntOff(void);

int main(void)
{
    initSystemClock_40MHz();
    enablePeripheralsClock();
    initPorts(resultReady);
    initI2c();
    initTimer1mSec(timerHandler);
    initUartPrintf();
    
    testTmp006init = tmp006_init(&senzor, TMP006_PIN_LOW, TMP006_PIN_LOW);
    
    UARTprintf("~~~TEST~~~ \n");
    
    runTest(test_readManufId, "Read manufacturer ID");
    
    runTest(test_writeIntoConfig, "Writing into config register");
    
    //test conversion rate with interrupt enabled
    runTest(test_readTemp1convPerSecIntOn, "Check 1 conversion per second rate with interrupt enabled (wait)");
    runTest(test_readTemp2convPerSecIntOn, "Check 2 conversion per second rate with interrupt enabled (wait)");
    runTest(test_readTemp4convPerSecIntOn, "Check 4 conversion per second rate with interrupt enabled (wait)");
    runTest(test_readTemp05convPerSecIntOn, "Check 0.5 conversion per second rate with interrupt enabled (wait)");
    runTest(test_readTemp025convPerSecIntOn, "Check 0.25 conversion per second rate with interrupt enabled (wait)");
    
    //test conversion rate with interrupt disabled
    runTest(test_readTemp1convPerSecIntOff, "Check 1 conversion per second rate with interrupt disabled (wait)");
    runTest(test_readTemp2convPerSecIntOff, "Check 2 conversion per second rate with interrupt disabled (wait)");
    runTest(test_readTemp4convPerSecIntOff, "Check 4 conversion per second rate with interrupt disabled (wait)");
    runTest(test_readTemp05convPerSecIntOff, "Check 0.5 conversion per second rate with interrupt disabled (wait)");
    runTest(test_readTemp025convPerSecIntOff, "Check 0.25 conversion per second rate with interrupt disabled (wait)");
    
    //test power down operation mode
    runTest(test_powerDownModeIntOn, "Check power down mode with interrupt enabled (wait)");
    runTest(test_powerDownModeIntOff, "Check power down mode with interrupt disabled (wait)");
    
    tmp006_resetDevice(&senzor);
    
    while(1)
    { 
        if (resultReadyFlag)
        {   
            tmp006_readTemp(&senzor, &temprature);
            tempInC = ((float)temprature * 0.03125);
            resultReadyFlag = 0;
        }
        
        if ((tempInC > 18) && (tempInC < 26))
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); //LED ON
        }
        else 
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); //LED OFF
        }   
    }
    return 0;
} //MAIN


bool checkTemperatureValue()
{
    if (resultReadyFlag)
    {   
        tmp006_readTemp(&senzor, &temprature);
        tempInC = ((float)temprature * 0.03125);
        resultReadyFlag = 0;
    }
    
    if ((tempInC < 18) && (tempInC > 26))
    {
        return false;
    }
    return true;
}


uint16_t readReg(TMP006_Device *dev, uint8_t addr)
{
    uint16_t Value, status;
    status = tmp006_read(dev, addr, &Value);
    if (status != 0)
    {
        return status;
    }
    return Value;
}

void resultReady(void)
{
    GPIOIntClear(GPIO_PORTA_BASE, GPIO_INT_PIN_2);
    resultCounter++ ;
    resultReadyFlag = 1;
}

//function is called every milisecond
void timerHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    msCounter++ ;
}


void runTest(bool (*testCase)(void), const char *nameOfTest)
{
    UARTprintf("Test case: %s",nameOfTest);
    bool success = testCase();
    UARTprintf(success ? " -> PASS \n" : " -> FAIL \n");
}

bool test_readManufId(void)
{
    uint16_t regValue;
    regValue = readReg(&senzor, TMP006_MANUFACTURER_ID);
    TMP006_TEST(regValue != TMP006_MANUF_ID_VALUE);
    return true;
}

bool test_writeIntoConfig(void)
{
    //TEST OF CONVERSION RATE CONFIGURATION
    for(uint16_t i = 0; i <= (TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC >> 9) ; i++)
    {
        tmp006_configConvRate(&senzor, (i << 9));
        TMP006_TEST_ASSERT(TMP006_CR_MASK, (i << 9));   //
    }
     
    //Test of DRDY pin mode
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    TMP006_TEST_ASSERT(TMP006_DRDY_EN_MASK, TMP006_DRDY_PIN_OFF);
    
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    TMP006_TEST_ASSERT(TMP006_DRDY_EN_MASK, TMP006_DRDY_PIN_ON);
    
    //Test of operation mode
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    TMP006_TEST_ASSERT(TMP006_MOD_MASK, TMP006_POWER_DOWN);
    
    tmp006_operationMode(&senzor, TMP006_CONTINUOUS_CONVERSION);
    TMP006_TEST_ASSERT(TMP006_MOD_MASK, TMP006_CONTINUOUS_CONVERSION);
    
    //Reset device test
    tmp006_resetDevice(&senzor);
    TMP006_TEST_ASSERT(0xFFFF, 0x7400); //default value of config reg after reset is 0x7400 
    
    return true;
}

bool test_readTemp1convPerSecIntOn(void)
{
    bool tempValue;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 5)
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
    }
    
    if(msCounter > (5.05 * 1000))
    {
        return false;
    }
    
    return true;  
}

bool test_readTemp2convPerSecIntOn(void)
{
    bool tempValue;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 10)
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
    }
    
    if(msCounter > (5.05 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp4convPerSecIntOn(void)
{
    bool tempValue;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 20)
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
    }
    
    if (msCounter > (5.05 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp05convPerSecIntOn(void)
{
    bool tempValue;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 5)
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
    }
    
    if (msCounter > (10.05 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp025convPerSecIntOn(void)
{
    bool tempValue;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 3)
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
    }
    
    if (msCounter > (12.1 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp1convPerSecIntOff(void)
{
    bool tempValue, resultReady;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 5)
    {
        tmp006_isResultReady(&senzor, &resultReady);
        if (resultReady)
        {
            resultCounter++;
            resultReadyFlag = 1;
            tempValue = checkTemperatureValue();
            if(!tempValue)
            {
                return false;
            }
        }
    }
    
    if (msCounter > (5.05 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp2convPerSecIntOff(void)
{
    bool tempValue, resultReady;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 10)
    {
        tmp006_isResultReady(&senzor, &resultReady);
        if (resultReady)
        {
            resultCounter++;
            resultReadyFlag = 1;
            tempValue = checkTemperatureValue();
            if(!tempValue)
            {
                return false;
            }
        }
    }
    
    if (msCounter > (5.05 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp4convPerSecIntOff(void)
{
    bool tempValue, resultReady;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 20)
    {
        tmp006_isResultReady(&senzor, &resultReady);
        if (resultReady)
        {
            resultCounter++;
            resultReadyFlag = 1;
            tempValue = checkTemperatureValue();
            if(!tempValue)
            {
                return false;
            }
        }
    }
    
    if (msCounter > (5.05 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp05convPerSecIntOff(void)
{
    bool tempValue, resultReady;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 5)
    {
        tmp006_isResultReady(&senzor, &resultReady);
        if (resultReady)
        {
            resultCounter++;
            resultReadyFlag = 1;
            tempValue = checkTemperatureValue();
            if(!tempValue)
            {
                return false;
            }
        }
    }
    
    if (msCounter > (10.05 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_readTemp025convPerSecIntOff(void)
{
    bool tempValue, resultReady;
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (resultCounter < 3)
    {
        tmp006_isResultReady(&senzor, &resultReady);
        if (resultReady)
        {
            resultCounter++;
            resultReadyFlag = 1;
            tempValue = checkTemperatureValue();
            if(!tempValue)
            {
                return false;
            }
        }
    }
    
    if (msCounter > (12.1 * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_powerDownModeIntOn(void)
{
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (msCounter < (5 * 1000))
    {
    }
    
    if(resultCounter > 0)
    {
        return false;
    }
    
    return true;
}

bool test_powerDownModeIntOff(void)
{
    bool tempValue, resultReady;
    
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    
    resultCounter = 0;
    msCounter = 0;
    
    while (msCounter < (5 * 1000))
    {
        tmp006_isResultReady(&senzor, &resultReady);
        if (resultReady)
        {
            resultCounter++;
            resultReadyFlag = 1;
            tempValue = checkTemperatureValue();
            if(!tempValue)
            {
                return false;
            }
        }
    }
    
    if(resultCounter > 0)
    {
        return false;
    }
    
    return true;
}
