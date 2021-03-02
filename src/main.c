/**
* @file main.c
* @brief Test of tmp006 driver
*
* @author Zarko Milojicic
*/

#include "tm4c_init.h"
#include "tmp006.h"

/**
* @brief Helper macro for parametar checking
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
* @brief Helper macro for checking of CONFIG register
*/    
#define TMP006_TEST_ASSERT(mask,checkValue)                      \
    do                                                           \
    {                                                            \
       uint16_t valueOfReg = (readReg(&senzor, TMP006_CONFIG));  \
       valueOfReg &= mask;                                       \
       TMP006_TEST(valueOfReg != checkValue);                    \
    }while(0)                                                    \
    
/**
* @brief Helper macro for test running
* Print PASS if test was success or FAIL if not
*/
#define RUN_TEST(testName, testCase, ...)                    \
    do                                                       \
    {                                                        \
        UARTprintf("Test case: %s", (testName));             \
        bool success = (testCase)(__VA_ARGS__);              \
        UARTprintf(success ? " -> PASS \n" : " -> FAIL \n"); \
    } while (0)                                              \
    
    
/** @brief counter of miliseconds, incremented in timer handler*/    
static volatile uint32_t msCounter = 0; 
/** @brief counter of received results */    
static volatile uint32_t resultCounter = 0;
/** @brief flag is set when result is received */
static volatile uint8_t resultReadyFlag = 0;
    
static TMP006_Device senzor = {
        .i2cRead = i2cRead,
        .i2cWrite = i2cWrite   
    };

/**
* @brief function for system set up before every test
*/    
void setUp(void)
{
    msCounter = 0;
    resultCounter = 0;
    resultReadyFlag = 0;
}
    
/**
* @brief Check if temperature is between 18 and 26 celsius degrees.
*
* @return true if temperature is between 18 and 26 celsius degrees, false if not.
*/
bool checkTemperatureValue(void);

/**
* @brief handler for timer, 1 ms timer
*/
void timerHandler(void);

/**
* @brief handler for edge interrupt on PORTA2
*
* Announce that result is ready via flag.
*/
void resultReady(void);

/**
* @brief Read value of specified register
* 
* Read 16-bit value of register with specified address.
*
* @param dev Pointer to the TMP006 device structure
* @param addr address of register you want to read
* @return 16-bit value of register.
*/    
uint16_t readReg(TMP006_Device *dev, uint8_t addr);

/**
* @brief test if values of commands are written into config register
*
* @return true if test success or false if not
*/
bool test_writeIntoConfig(void);

/**
* @brief reading manufacturer id from device
*
* @return true if test success or false if not
*/
bool test_readManufId(void);

/**
* @brief test of different conversion rate with enabled interrupt pin
*
* @param convRate speed of conversion 
* @param numberOfResults number of result you want to check at determined rate
* @return true if test success or false if not
*/
bool test_customConvRateIntOn(enum TMP006_ConversionRate convRate, uint16_t numberOfResults);

/**
* @brief test of different conversion rate with disabled interrupt pin
*
* @param convRate speed of conversion 
* @param numberOfResults number of result you want to check at determined rate
* @return true if test success or false if not
*/
bool test_customConvRateIntOff(enum TMP006_ConversionRate convRate, uint16_t numberOfResults);

/**
* @brief test power-down operation mode with interrupt enabled
* @return true if test success or false if not
*/
bool test_powerDownModeIntOn(void);

/**
* @brief test power-down operation mode with interrupt disabled
* @return true if test success or false if not
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
    
    tmp006_init(&senzor, TMP006_PIN_LOW, TMP006_PIN_LOW);
    
    UARTprintf("~~~TEST~~~ \n");
    
    RUN_TEST("Read manufacturer ID", test_readManufId);
    
    RUN_TEST("Writing into config register", test_writeIntoConfig);
    
    //test conversion rate with interrupt enabled
    RUN_TEST("Check 1 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_1_CONV_PER_SEC, 5);
    RUN_TEST("Check 2 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_2_CONV_PER_SEC, 5);
    RUN_TEST("Check 4 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_4_CONV_PER_SEC, 5);
    RUN_TEST("Check 0.5 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC, 3);
    RUN_TEST("Check 0.25 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC, 2);
    
    //test conversion rate with interrupt disabled
    RUN_TEST("Check 1 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_1_CONV_PER_SEC, 5);
    RUN_TEST("Check 2 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_2_CONV_PER_SEC, 5);
    RUN_TEST("Check 4 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_4_CONV_PER_SEC, 5);
    RUN_TEST("Check 0.5 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC, 3);
    RUN_TEST("Check 0.25 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff,TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC, 2);
    
    //test power down operation mode
    RUN_TEST("Check power down mode with interrupt enabled (wait)", test_powerDownModeIntOn);
    RUN_TEST( "Check power down mode with interrupt disabled (wait)", test_powerDownModeIntOff);
    
    tmp006_resetDevice(&senzor);

    return 0;
} //MAIN


bool checkTemperatureValue(void)
{
    int16_t temprature = 0;
    float tempInC = 0;
    
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

bool test_customConvRateIntOn(enum TMP006_ConversionRate convRate, uint16_t numberOfResults)
{
    bool tempValue;
    
    setUp();
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, convRate);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    
    float multipleFactor = 0;
    
    if (convRate == TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC)
    {
        multipleFactor = 4;
    }
    else if (convRate == TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC)
    {
        multipleFactor = 2;
    }
    else if (convRate == TMP006_CONVERSION_RATE_1_CONV_PER_SEC)
    {
        multipleFactor = 1;
    }
    else if (convRate == TMP006_CONVERSION_RATE_2_CONV_PER_SEC)
    {
        multipleFactor = 0.5;
    }
    else if (convRate == TMP006_CONVERSION_RATE_4_CONV_PER_SEC)
    {
        multipleFactor = 0.25;
    }
    
    while (resultCounter < numberOfResults)
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
    }
    
    if(msCounter > (((numberOfResults * multipleFactor) + 0.05) * 1000))
    {
        return false;
    }
    
    return true; 
}



bool test_customConvRateIntOff(enum TMP006_ConversionRate convRate, uint16_t numberOfResults)
{
    bool tempValue, resultReady;
    
    setUp();
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, convRate);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    
    float multipleFactor = 0;
    
    if (convRate == TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC)
    {
        multipleFactor = 4;
    }
    else if (convRate == TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC)
    {
        multipleFactor = 2;
    }
    else if (convRate == TMP006_CONVERSION_RATE_1_CONV_PER_SEC)
    {
        multipleFactor = 1;
    }
    else if (convRate == TMP006_CONVERSION_RATE_2_CONV_PER_SEC)
    {
        multipleFactor = 0.5;
    }
    else if (convRate == TMP006_CONVERSION_RATE_4_CONV_PER_SEC)
    {
        multipleFactor = 0.25;
    }
    
    while (resultCounter < numberOfResults)
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
    if (msCounter > (((numberOfResults * multipleFactor) + 0.05) * 1000))
    {
        return false;
    }
    
    return true; 
}

bool test_powerDownModeIntOn(void)
{
    setUp();
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    
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
    
    setUp();
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    
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
