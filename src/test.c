/**
* @file test.c
* @brief test cases for tmp006 driver
*
* @author Zarko Milojicic
*/

#include "tmp006/tmp006.h"
#include "test.h"

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

bool test_readManufId(void)
{
    uint16_t regValue;
    int status = tmp006_read(&senzor, TMP006_MANUFACTURER_ID, &regValue);
    
    TEST_ASSERT(status == 0);
    TEST_ASSERT(regValue == TMP006_MANUF_ID_VALUE);
    
    return true;
}

bool test_writeIntoConfig(void)
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

bool test_customConvRateIntOn(enum TMP006_ConversionRate convRate)
{
    bool tempValue;

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

bool test_customConvRateIntOff(enum TMP006_ConversionRate convRate)
{
    bool tempValue, resultReady;
    
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

bool test_powerDownModeIntOn(void)
{
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

bool test_powerDownModeIntOff(void)
{
    bool tempValue, resultReady;
    
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

