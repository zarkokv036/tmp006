/**
* @file test_framework.c
* @brief test cases for tmp006
*
* @author Zarko Milojicic
*/

#include <stdint.h>
#include <stdbool.h>
#include "test.h"

/** @brief counter of miliseconds, incremented in timer handler*/    
volatile uint32_t msCounter = 0; 
/** @brief counter of received results */    
volatile uint32_t resultCounter = 0;
/** @brief when set calculation can be performed*/
volatile uint8_t resultReadyFlag = 0;

TMP006_Device senzor = {
        .i2cRead = platform_i2cRead,
        .i2cWrite = platform_i2cWrite   
    };
   
    
void pinInterruptHandler(void)
{
    resultCounter++ ;
    resultReadyFlag = 1;
}

void timerHandler(void)
{
    msCounter++ ;
}    
    
void setUp(void)
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

bool test_customConvRateIntOff(enum TMP006_ConversionRate convRate)
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

bool test_powerDownModeIntOn(void)
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

bool test_powerDownModeIntOff(void)
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

void test_init(void)
{
    platform_configure1msInterrupt(timerHandler);

    platform_configureInterruptPin(pinInterruptHandler);
}