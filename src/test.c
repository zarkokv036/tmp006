
#include <stdint.h>
#include <stdbool.h>

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
       uint16_t valueOfReg;                                      \
       int status = tmp006_read(&senzor, TMP006_CONFIG, &valueOfReg);         \
       if (status != 0)                                              \
       {                                                         \
           return false;                                         \
       }                                                         \
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
/** @brief flag is set when result is received from sensor*/
static volatile uint8_t resultReadyFlag = 0;

bool flag1 = false, flag2 = false;
    
static TMP006_Device senzor = {
        .i2cRead = i2cRead,
        .i2cWrite = i2cWrite   
    };


bool checkTemperatureValue(void)
{
    uint32_t msCounterSnap = msCounter;
    uint16_t seconds = 5;
    
    while(!resultReadyFlag)
    {
        if(msCounter > (msCounterSnap + seconds * 1000))
        {
            return false;
        }
    }
    
    int16_t temprature = 0;
    int16_t status = tmp006_readTemp(&senzor, &temprature);
    TMP006_TEST(status != 0); 
    
    const float tempInC = (float)temprature * 0.03125f;
    resultReadyFlag = 0;
 
    return ((tempInC >= 18) && (tempInC <= 26));
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
    int status = tmp006_read(&senzor, TMP006_MANUFACTURER_ID, &regValue);
    
    TMP006_TEST(status != 0);
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
    
    uint16_t expectedNumberOfResults = 2;
    uint16_t snapshot;
    
    while (msCounter < (((expectedNumberOfResults * multipleFactor) + 0.05) * 1000)) //(resultCounter < numberOfResults)
    {
        tempValue = checkTemperatureValue();
        if (!tempValue)
        {
            return false;
        }
        snapshot = resultCounter;
    }
    
    if (snapshot != resultCounter)
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