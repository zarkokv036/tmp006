/**
* @file test.c
* @brief test cases for tmp006 driver
*
* @author Zarko Milojicic
*/


#include <stdint.h>
#include <stdbool.h>
#include "tmp006/tmp006.h"
#include "platform.h"

/**
* @brief Helper macro for parametar checking
*/    
#define TEST_ASSERT(expr)                 \
    do                                   \
    {                                    \
        bool e = expr;                   \
        if (!e)                          \
        {                                \
            return false;                \
        }                                \
                                         \
    } while (0)
    

/**
* @brief Helper macro for test running
* Print PASS if test was success or FAIL if not
*/
#define RUN_TEST(testName, testCase, ...)                    \
    do                                                       \
    {                                                        \
        PRINTF("Test case: %s", (testName));                 \
        bool success = (testCase)(__VA_ARGS__);              \
        PRINTF(success ? " -> PASS \n" : " -> FAIL \n");     \
    } while (0)                                              \
    
    
/** @brief counter of miliseconds, incremented in timer handler*/    
static volatile uint32_t msCounter = 0; 
/** @brief counter of received results */    
static volatile uint32_t resultCounter = 0;
/** @brief when set calculation can be performed*/
static volatile uint8_t resultReadyFlag = 0;
    
static TMP006_Device senzor = {
        .i2cRead = hal_i2cRead,
        .i2cWrite = hal_i2cWrite   
    };

/**
* @brief handler for edge interrupt on pin.
* Announce that result is ready via resultReadyFlag and counts the results.
*/
static void pinInterruptHandler(void)
{
    resultCounter++ ;
    resultReadyFlag = 1;
}

/**
* @brief handler for timer, 1 ms timer
*/
static void timerHandler(void)
{
    msCounter++ ;
}    
    
/**
* @brief function for system set up before every test
*/      
static void setUp(void)
{
    msCounter = 0;
    resultCounter = 0;
    resultReadyFlag = 0;
}

/**
* @brief check value at required position in the CONFIG register.
*
* @param mask Position of values in config register you want to check.
* @param checkValue Value you expect to be in register at required position.
* @return true if CheckValue is at required position, false if it's not.
*/
static bool checkConfigReg(uint16_t mask, uint16_t checkValue)
{
    uint16_t valueOfReg;
    
    int status = tmp006_read(&senzor, TMP006_CONFIG, &valueOfReg);
    
    if (status != 0)                     
    {                                    
        return false;                    
    } 
    
    valueOfReg &= mask;                  
    TEST_ASSERT(valueOfReg == checkValue);
    
    return true;
}

/**
* @brief Check if temperature is between 18 and 26 celsius degrees.
*
* @return true if temperature is between 18 and 26 celsius degrees, false if not.
*/
static bool checkTemperatureValue(void)
{
    uint32_t msCounterSnap = msCounter;
    uint16_t seconds = 5;
    
    //calculation of temp is performed only if resultReadyFlag is set
    while(!resultReadyFlag)
    {
        if(msCounter > (msCounterSnap + seconds * 1000))
        {
            return false;
        }
    }
    
    int16_t temprature = 0;
    int16_t status = tmp006_readTemp(&senzor, &temprature);
    TEST_ASSERT(status == 0); 
    
    const float tempInC = (float)temprature * 0.03125f;
    resultReadyFlag = 0;
 
    return ((tempInC >= 18) && (tempInC <= 26));
}

/**
* @brief reading manufacturer id from device
*
* @return true if test success or false if not
*/
static bool test_readManufId(void)
{
    uint16_t regValue;
    int status = tmp006_read(&senzor, TMP006_MANUFACTURER_ID, &regValue);
    
    TEST_ASSERT(status == 0);
    TEST_ASSERT(regValue == TMP006_MANUF_ID_VALUE);
    
    return true;
}

/**
* @brief test if values of commands are written into config register
*
* @return true if test success or false if not
*/
static bool test_writeIntoConfig(void)
{
    //TEST OF CONVERSION RATE CONFIGURATION
    for(uint16_t i = 0; i <= (TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC >> 9) ; i++)
    {
        tmp006_configConvRate(&senzor, (i << 9));
        TEST_ASSERT(checkConfigReg(TMP006_CR_MASK, (i << 9)));   //
    }
     
    //Test of DRDY pin mode
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    TEST_ASSERT(checkConfigReg(TMP006_DRDY_EN_MASK, TMP006_DRDY_PIN_OFF));
    
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    TEST_ASSERT(checkConfigReg(TMP006_DRDY_EN_MASK, TMP006_DRDY_PIN_ON));
    
    //Test of operation mode
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    TEST_ASSERT(checkConfigReg(TMP006_MOD_MASK, TMP006_POWER_DOWN));
    
    tmp006_operationMode(&senzor, TMP006_CONTINUOUS_CONVERSION);
    TEST_ASSERT(checkConfigReg(TMP006_MOD_MASK, TMP006_CONTINUOUS_CONVERSION));
    
    //Reset device test
    tmp006_resetDevice(&senzor);
    TEST_ASSERT(checkConfigReg(0xFFFF, 0x7400)); //default value of config reg after reset is 0x7400 
    
    return true;
}

/**
* @brief calculation of mulitple factor
* Multiple factor is used for calculation of time needed to get all results.
*/
static float conversionRateToFloat(enum TMP006_ConversionRate convRate)
{
    switch (convRate)
    {
        case TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC: return 0.25f;
        case TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC:  return 0.5f;
        case TMP006_CONVERSION_RATE_1_CONV_PER_SEC:    return 1.0f;
        case TMP006_CONVERSION_RATE_2_CONV_PER_SEC:    return 2.0f;
        case TMP006_CONVERSION_RATE_4_CONV_PER_SEC:    return 4.0f;  
    }
}

/**
* @brief test of different conversion rate with enabled interrupt pin
*
* @param convRate speed of conversion 
* @return true if test success or false if not
*/
static bool test_customConvRateIntOn(enum TMP006_ConversionRate convRate)
{
    bool tempValue;

    setUp();
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, convRate);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    
    float multipleFactor = 1.0f / conversionRateToFloat(convRate);
    
    //number of results you want to receive from sensor
    uint16_t expectedNumberOfResults = 2;
    uint16_t savedResultCounter = 0;
    
    while (msCounter < (((expectedNumberOfResults * multipleFactor) + 0.05) * 1000)) 
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
        savedResultCounter = resultCounter;
    }
    
    if (savedResultCounter != resultCounter)
    {
        return false;
    }
    
    return true; 
}

/**
* @brief test of different conversion rate with disabled interrupt pin
*
* @param convRate speed of conversion
* @return true if test success or false if not
*/
static bool test_customConvRateIntOff(enum TMP006_ConversionRate convRate)
{
    bool tempValue, resultReady;
    
    setUp();
    tmp006_resetDevice(&senzor);
    tmp006_configConvRate(&senzor, convRate);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    
    float multipleFactor = 1.0f / conversionRateToFloat(convRate);
    
    uint16_t expectedNumberOfResults = 2;
    
    while (msCounter < (((expectedNumberOfResults * multipleFactor) + 0.05) * 1000))
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
    if (expectedNumberOfResults != resultCounter)
    {
        return false;
    }
    
    return true; 
}

/**
* @brief test power-down operation mode with interrupt enabled
*
* @return true if test success or false if not
*/
static bool test_powerDownModeIntOn(void)
{
    setUp();
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_ON);
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    
    uint16_t secToWait = 2;
    
    while (msCounter < (secToWait * 1000))
    {
    }
    
    if(resultCounter > 0)
    {
        return false;
    }
    
    return true;
}

/**
* @brief test power-down operation mode with interrupt disabled
*
* @return true if test success or false if not
*/
static bool test_powerDownModeIntOff(void)
{
    bool tempValue, resultReady;
    
    setUp();
    tmp006_configConvRate(&senzor, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    tmp006_drdyPinConfig(&senzor, TMP006_DRDY_PIN_OFF);
    tmp006_operationMode(&senzor, TMP006_POWER_DOWN);
    
    uint16_t secToWait = 2;
    
    while (msCounter < (secToWait * 1000))
    {
        tmp006_isResultReady(&senzor, &resultReady);
        if (resultReady)
        {
            return false;  
        }
    }

    return true;
}



void test_run(void)
{
    tmp006_init(&senzor, TMP006_PIN_LOW, TMP006_PIN_LOW);
    
    PRINTF("~~~TEST~~~ \n");
    
    RUN_TEST("Read manufacturer ID", test_readManufId);
    
    RUN_TEST("Writing into config register", test_writeIntoConfig);
    
    //test conversion rate with interrupt enabled
    RUN_TEST("Check 1 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    RUN_TEST("Check 2 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
    RUN_TEST("Check 4 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
    RUN_TEST("Check 0.5 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
    RUN_TEST("Check 0.25 conversion per second rate with interrupt enabled (wait)", test_customConvRateIntOn, TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
    
    //test conversion rate with interrupt disabled
    RUN_TEST("Check 1 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_1_CONV_PER_SEC);
    RUN_TEST("Check 2 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_2_CONV_PER_SEC);
    RUN_TEST("Check 4 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_4_CONV_PER_SEC);
    RUN_TEST("Check 0.5 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff, TMP006_CONVERSION_RATE_0_5_CONV_PER_SEC);
    RUN_TEST("Check 0.25 conversion per second rate with interrupt disabled (wait)", test_customConvRateIntOff,TMP006_CONVERSION_RATE_0_25_CONV_PER_SEC);
    
    //test power down operation mode
    RUN_TEST("Check power down mode with interrupt enabled (wait)", test_powerDownModeIntOn);
    RUN_TEST( "Check power down mode with interrupt disabled (wait)", test_powerDownModeIntOff);
    
    tmp006_resetDevice(&senzor);
    
    PRINTF("~~~TEST END~~~ \n");
}

void test_init(void)
{
    hal_configure1msInterrupt(timerHandler);

    hal_configureInterruptPin(pinInterruptHandler);
}

